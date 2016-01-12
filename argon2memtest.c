#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t Node;
#define NodeNull UINT32_MAX

struct NodeSt {
    Node firstChild, lastChild, prevSibling, nextSibling, parent;
};

static struct NodeSt *Nodes;

static void addToList(Node parent, Node child) {
    Nodes[child].nextSibling = Nodes[parent].firstChild;
    Nodes[child].prevSibling = NodeNull;
    if(Nodes[parent].firstChild == NodeNull) {
        Nodes[parent].firstChild = child;
        Nodes[parent].lastChild = child;
    } else {
        Nodes[Nodes[parent].firstChild].prevSibling = child;
    }
    Nodes[parent].firstChild = child;
    Nodes[child].parent = parent;
}

static void removeFromList(Node parent, Node child) {
    Node prev = Nodes[child].prevSibling;
    Node next = Nodes[child].nextSibling;
    if(prev != NodeNull) {
        Nodes[prev].nextSibling = next;
    } else {
        Nodes[parent].firstChild = next;
    }
    if(next != NodeNull) {
        Nodes[next].prevSibling = prev;
    } else {
        Nodes[parent].lastChild = prev;
    }
    Nodes[child].parent = NodeNull;
    Nodes[child].nextSibling = NodeNull;
    Nodes[child].prevSibling = NodeNull;
}

static Node findPrevNode(uint32_t i) {
    uint64_t prev = rand();
    prev = (prev*prev)/RAND_MAX; // Square the distribution
    prev = i - (((i-1)*prev)/RAND_MAX) - 1;
    return prev;
}

int main(int argc, char **argv) {
    if(argc > 3 || argc < 2) {
        fprintf(stderr, "Usage: argon2memtest [-v] numBlocks\n"
            "    -v -- Verblose debug output\n");
        return 1;
    }
    bool debug = false;
    uint32_t numBlocks;
    if(argc == 3) {
        if(strcmp(argv[1], "-v")) {
            fprintf(stderr, "Bad argument\n");
            return 1;
        }
        debug = true;
        numBlocks = atoi(argv[2]);
    } else {
        numBlocks = atoi(argv[1]);
    }
    if(numBlocks <= 0) {
        fprintf(stderr, "Bad numBlocks\n");
        return 1;
    }
    if(debug) printf("RAND_MAX: %x\n", RAND_MAX);
    Nodes = calloc(numBlocks, sizeof(struct NodeSt));
    memset(Nodes, 0xff, (uint64_t)numBlocks*sizeof(struct NodeSt));
    uint32_t i;
    for(i = 1; i < numBlocks; i++) {
        Node prev = findPrevNode(i);
        addToList(prev, i);
    }
    uint32_t usedBlocks = 1;
    uint32_t maxBlocks = 1;
    for(i = 1; i < numBlocks; i++) {
        // The new block only adds memory if it is used later
        if(Nodes[i].firstChild != NodeNull) {
            if(debug) printf("Adding node %u, which accesses %u\n", i, Nodes[i].parent);
            usedBlocks++;
        }
        if(usedBlocks > maxBlocks) {
            maxBlocks = usedBlocks;
        }
        // Throw away the block it referenced if no longer needed
        Node parent = Nodes[i].parent;
        removeFromList(parent, i);
        if(Nodes[parent].firstChild == NodeNull) {
            // No longer need this node
            if(debug) printf("Node %u no longer needed\n", parent);
            usedBlocks--;
        }
    }
    printf("Max blocks used %u out of %u.  Memory reduction %.2fX\n",
        maxBlocks, numBlocks, (float)numBlocks/maxBlocks);
    return 0;
}
