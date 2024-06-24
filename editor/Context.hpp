namespace jed {
class Context {
public:
    int width;
    int height;
    int tab_width = 4;
    float font_size = 24.0f;
    float gutter_width = font_size + 1.0f;

    static Context& get();

private:
    Context() = default;
    ~Context() = default;
};
} // namespace jed
