#ifndef __BTREE_INTERNAL_NODE_HPP__
#define __BTREE_INTERNAL_NODE_HPP__

#include "utils.hpp"
#include "btree/node.hpp"

//Note: This struct is stored directly on disk.  Changing it invalidates old data.
struct btree_internal_pair {
    block_id_t lnode;
    btree_key key;
};


//Note: This struct is stored directly on disk.  Changing it invalidates old data.
struct btree_internal_node : public btree_node {
    uint16_t npairs;
    uint16_t frontmost_offset;
    uint16_t pair_offsets[0];
};

class internal_key_comp;

typedef btree_internal_node internal_node_t;

class internal_node_handler : public node_handler {
    friend class internal_key_comp;
    public:
    static void init(btree_internal_node *node);
    static void init(btree_internal_node *node, btree_internal_node *lnode, uint16_t *offsets, int numpairs);

    static block_id_t lookup(btree_internal_node *node, btree_key *key);
    static int insert(btree_internal_node *node, btree_key *key, block_id_t lnode, block_id_t rnode);
    static bool remove(btree_internal_node *node, btree_key *key);
    static void split(btree_internal_node *node, btree_internal_node *rnode, btree_key *median);
    static void merge(btree_internal_node *node, btree_internal_node *rnode, btree_key *key_to_remove, btree_internal_node *parent);
    static void level(btree_internal_node *node, btree_internal_node *rnode, btree_key *key_to_replace, btree_key *replacement_key, btree_internal_node *parent);
    static int sibling(btree_internal_node *node, btree_key *key, block_id_t *sib_id);
    static void update_key(btree_internal_node *node, btree_key *key_to_replace, btree_key *replacement_key);
    static int nodecmp(btree_internal_node *node1, btree_internal_node *node2);
    static bool is_full(btree_internal_node *node);
    static bool is_underfull(btree_internal_node *node);
    static bool is_underfull_or_min(btree_internal_node *node); // TODO: rename
    static bool is_singleton(btree_internal_node *node);

    static void validate(btree_internal_node *node);

    protected:
    static size_t pair_size(btree_internal_pair *pair);
    static btree_internal_pair *get_pair(btree_internal_node *node, uint16_t offset);
    static void delete_pair(btree_internal_node *node, uint16_t offset);
    static uint16_t insert_pair(btree_internal_node *node, btree_internal_pair *pair);
    static uint16_t insert_pair(btree_internal_node *node, block_id_t lnode, btree_key *key);
    static int get_offset_index(btree_internal_node *node, btree_key *key);
    static void delete_offset(btree_internal_node *node, int index);
    static void insert_offset(btree_internal_node *node, uint16_t offset, int index);
    static void make_last_pair_special(btree_internal_node *node);
    static bool is_equal(btree_key *key1, btree_key *key2);
};

class internal_key_comp {
    btree_internal_node *node;
    btree_key *key;
    public:
    internal_key_comp(btree_internal_node *_node) : node(_node), key(NULL)  { };
    internal_key_comp(btree_internal_node *_node, btree_key *_key) : node(_node), key(_key)  { };
    bool operator()(const uint16_t offset1, const uint16_t offset2) {
        btree_key *key1 = offset1 == 0 ? key : &internal_node_handler::get_pair(node, offset1)->key;
        btree_key *key2 = offset2 == 0 ? key : &internal_node_handler::get_pair(node, offset2)->key;
        int cmp;
        if (key1->size == 0 && key2->size == 0) //check for the special end pair
            cmp = 0;
        else if (key1->size == 0)
            cmp = 1;
        else if (key2->size == 0)
            cmp = -1;
        else
            cmp = sized_strcmp(key1->contents, key1->size, key2->contents, key2->size);

        return cmp < 0;
    }
};



#endif // __BTREE_INTERNAL_NODE_HPP__
