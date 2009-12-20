// build: g++ -Wall -lm -lusb -o analyze_tree_bin analyze_tree_bin.cc tubecable.c 
// usage: analyze_tree_bin 8 | tail -n +2 | dot -Tpng -otree.png

#include "tubecable.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>

typedef struct {
	uint8_t size;
	uint32_t seq;
} dl_huffman_entry;


// The userspace Huffman table.
extern dl_huffman_entry dl_huffman_compact[DL_HUFFMAN_SIZE];


// binary tree
struct node {

	node(): key(0), value(0) { c[0] = 0; c[1] = 0; }

	node* c[2];
	int key;
	int value;
};


node* search( node* root, int key, int len ) {
	int bit = 0;
	while (root && (bit < len)) {
		root = root->c[(key >> bit) & 1];
		bit++;
	}
	return root;
}


node* insert( node* root, int key, int len, int shift, int value ) {

	int origkey = key; 
	key = key >> shift;

	node* prev = search( root, key, len );
	if (prev) return prev;

	node* next = root->c[key & 1];
	if (!next) {
		next = new node();
		root->c[key & 1] = next;
	}

	if (len == 1) {
		next->key = origkey;
		next->value = value;
		return next;
	}

	return insert( next, origkey, len-1, shift+1, value );
}


void print( node* root, int depth ) {

	if (!root) return;

	if (depth == 0) std::cout << "tree: " << std::endl;

	for (int i = 0; i <= depth; i++) std::cout << "  ";
	std::cout << "key: " << root->key << " value: " << root->value;
	if (!root->c[0] && !root->c[1]) std::cout << " (final)";
	std::cout << std::endl;

	print( root->c[0], depth+1 );
	print( root->c[1], depth+1 );
}


void print_name( node* root ) {
	if (!root->c[0] && !root->c[1])
		std::cout << "\"" << root->value << "\"";
	else
		std::cout << "\"" << root << "\"";
}

void print_nodes( node* root ) {

	if (!root) return;

	print_name( root );
	std::cout << ";" << std::endl;

	print_nodes( root->c[0] );
	print_nodes( root->c[1] );
}

void print_edges( node* root ) {

	if (!root) return;

	if (root->c[0]) {
		print_name( root );
		std::cout << " -> ";
		print_name( root->c[0] );
		std::cout << std::endl;
	}

	if (root->c[1]) {
		print_name( root );
		std::cout << " -> ";
		print_name( root->c[1] );
		std::cout << std::endl;
	}

	print_edges( root->c[0] );
	print_edges( root->c[1] );
}





int main( int argc, const char* argv[] ) {

	dl_huffman_load_table( "tubecable_huffman.bin" );

	//node* head = new node(); head->bit = -1;
	node* root = new node();

	dl_huffman_entry* table = &(dl_huffman_compact[DL_HUFFMAN_COUNT]);

	// build the tree
	for (int i = -DL_HUFFMAN_COUNT; i <= DL_HUFFMAN_COUNT; i++) if (table[i].size <= atoi(argv[1])) {
		insert( root, table[i].seq, table[i].size, 0, i );
	}

	//print( root, 0 );

	std::cout << "digraph huffmantree {\n ranksep=1\n nodesep=1\n overlap=false" << std::endl;
	std::cout << " root = \"" << root << "\"" << std::endl;

	print_nodes( root );
	print_edges( root );

	std::cout << "}" << std::endl;
}
