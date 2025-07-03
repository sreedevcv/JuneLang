#include "Flatten.hpp"

#include "Chunk.hpp"
#include "Ir.hpp"
#include "OpCode.hpp"
#include "Utils.hpp"

#include <cstdint>
#include <iomanip>
#include <utility>
#include <vector>

static std::pair<
    std::vector<jl::Ir>,
    std::vector<uint32_t>>
remove_labels(
    const jl::Chunk& chunk,
    int offset,
    const std::unordered_map<std::string, int>& func_indices)
{
    const auto& irs = chunk.get_ir();
    const auto& lines = chunk.get_lines();

    std::vector<jl::Ir> new_irs;
    std::vector<uint32_t> new_lines;
    std::vector<uint32_t> label_locs;
    label_locs.resize(chunk.get_max_labels());

    for (int i = 0; i < irs.size(); i++) {
        if (irs[i].opcode() == jl::OpCode::LABEL) {
            label_locs[std::get<int>(irs[i].control().data)] = i + 1 + offset;
        }
    }

    for (int i = 0; i < irs.size(); i++) {
        const auto& ir = irs[i];

        // Skip LABELs
        if (ir.opcode() == jl::OpCode::LABEL) {
            continue;
        }
        // Backpatch JMPs
        if (ir.opcode() == jl::OpCode::JMP_UNLESS) {
            const auto& current_label = ir.jump().label;
            const int actual_loc = label_locs[std::get<int>(current_label)];
            auto new_ir = ir.jump();
            new_ir.label = actual_loc;
            new_irs.push_back({ new_ir });
        } else if (ir.opcode() == jl::OpCode::JMP_UNLESS) {
            const auto& current_label = ir.control().data;
            const int actual_loc = label_locs[std::get<int>(current_label)];
            auto new_ir = ir.control();
            new_ir.data = actual_loc;
            new_irs.push_back({ new_ir });
        } else if (ir.opcode() == jl::OpCode::CALL) {
            // Substitute the temp vvar that reprsents the func name with the index
            const auto& func_var = ir.control().data;
            const auto& func_name = chunk.get_variable_name_from_temp_var(std::get<jl::TempVar>(func_var).idx);
            const int func_index = func_indices.at(func_name);
            auto new_ir = ir.control();
            new_ir.data = func_index;
            new_irs.push_back({ new_ir });
        } else {
            new_irs.push_back(ir);
        }

        new_lines.push_back(lines[i]);
    }

    return { new_irs, new_lines };
}

static void backpatch_calls(
    std::vector<jl::Ir>& irs,
    const std::unordered_map<std::string, int>& func_locs,
    const std::unordered_map<std::string, int>& func_indices)
{
    const auto get_offset = [&](int idx) {
        for (const auto& [name, index] : func_indices) {
            if (idx == index) {
                return func_locs.at(name);
            }
        }

        unimplemented();
        return 0;
    };

    for (auto& ir : irs) {
        if (ir.opcode() == jl::OpCode::CALL) {
            const auto func_index = std::get<int>(ir.control().data);
            const auto offset = get_offset(func_index);
            std::get<jl::ControlIr>(ir.data).data = offset;
        }
    }
}

std::pair<
    std::vector<jl::Ir>,
    std::vector<uint32_t>>
jl::flatten(const std::map<std::string, jl::Chunk>& chunks)
{
    int offset = 0;
    int func_index = 0;
    std::vector<jl::Ir> new_irs;
    std::vector<uint32_t> new_lines;
    std::unordered_map<std::string, int> func_locs; // name and offset
    std::unordered_map<std::string, int> func_indices; // name and index

    for (const auto& [name, chunk] : chunks) {
        func_indices.insert({ name, func_index++ });
    }

    for (const auto& [name, chunk] : chunks) {
        const auto [irs, lines] = remove_labels(chunk, offset, func_indices);

        func_locs.insert({ name, offset });
        new_irs.append_range(irs);
        new_lines.append_range(lines);
        offset += irs.size();
    }

    backpatch_calls(new_irs, func_locs, func_indices);

    return { new_irs, new_lines };
}

std::ostream& jl::disassemble(
    std::ostream& out,
    const std::vector<jl::Ir>& irs,
    const std::vector<uint32_t>& lines)
{
    using namespace jl;

    uint32_t line = -1;

    for (int i = 0; i < irs.size(); i++) {
        // Print line number
        if (lines[i] != line) {
            line = lines[i];
            out << std::setfill('0') << std::setw(4) << line;
        } else {
            out << "  | ";
        }

        // Print ir index
        out << ' ';
        out << std::setfill('0') << std::setw(4) << i;

        // Print destination and opcode
        if (irs[i].type() == Ir::BINARY || irs[i].type() == Ir::UNARY) {
            out << std::setfill(' ') << std::setw(10) << to_string(irs[i].dest());
            out << " : ";
            out << std::setfill(' ') << std::setw(10) << jl::to_string(irs[i].opcode());
        } else {
            out << std::setfill(' ') << std::setw(10) << ' ';
            out << "   ";
            out << std::setfill(' ') << std::setw(10) << jl::to_string(irs[i].opcode());
        }

        switch (irs[i].type()) {
        case Ir::BINARY:
            out << std::setfill(' ') << std::setw(10) << to_string(irs[i].binary().op1);
            out << std::setfill(' ') << std::setw(10) << to_string(irs[i].binary().op2);
            break;
        case Ir::UNARY:
            out << std::setfill(' ') << std::setw(10) << to_string(irs[i].unary().operand);
            break;
        case Ir::CONTROL:
            out << std::setfill(' ') << std::setw(10) << "{" << to_string(irs[i].control().data) << "}";
            break;
        case Ir::JUMP:
            out << std::setfill(' ') << std::setw(10) << "{" << std::get<int>(irs[i].jump().label) << "}";
            out << std::setfill(' ') << std::setw(10) << to_string(irs[i].jump().data);
            break;
        default:
            unimplemented();
        }
        out << '\n';
    }

    return out;
}