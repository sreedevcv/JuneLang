# The Cooper-Harvey-Kennedy Algorithm for dominator tree

```
doms[start_node] = start_node
for each node b (except start_node):
    doms[b] = undefined

changed = True
while changed:
    changed = False
    for each node b in RPO (excluding start_node):
        # 1. Find the first processed predecessor
        new_idom = first_processed_predecessor(b)
        
        # 2. Intersect with all other processed predecessors
        for each other predecessor p of b:
            if doms[p] is not undefined:
                new_idom = intersect(p, new_idom)
        
        # 3. If IDom changed, update and keep going
        if doms[b] != new_idom:
            doms[b] = new_idom
            changed = True

def intersect(b1, b2):
    finger1 = b1
    finger2 = b2
    while finger1 != finger2:
        # If finger1 is "lower" in the RPO (smaller index), move it up
        while rpo_index[finger1] < rpo_index[finger2]:
            finger1 = doms[finger1]
        # If finger2 is "lower", move it up
        while rpo_index[finger2] < rpo_index[finger1]:
            finger2 = doms[finger2]
    return finger1

def first_processed_predecessor(b):
    for p in preds[b]:
        if doms[p] is not undefined:
            return p
    return undefined  # No processed predecessor found

```