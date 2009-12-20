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


struct node {

	node(): bit(0), key(0), value(0), bc(0) { c[0] = this; c[1] = this; }

	node* c[2];
	int bit;
	int key;

	int value;
	int bc;

};


node* search( node* root, int key ) {
	node* prev;
	do {
		prev = root;
		root = root->c[(key >> root->bit) & 1];
	} while (prev->bit < root->bit);
	return root;
}


node* insert( node* root, int key, int value ) {

	node* t = search( root, key );
	if (key == t->key) return t;

	int i = 0;
	while ( ((key >> i) & 1) == ((t->key >> i) & 1) ) i++;

	node* prev;
	do {
		prev = root;
		root = root->c[(key >> root->bit) & 1];
	} while ( (root->bit <= i) && (prev->bit < root->bit) );

	t = new node();

	t->bit = i;
	t->key = key;
	t->value = value;

	if ( ((key >> t->bit) & 1) == 0 ) {
		t->c[0] = t;
		t->c[1] = root;
	} else {
		t->c[0] = root;
		t->c[1] = t;
	}

	prev->c[((key >> prev->bit) & 1)] = t;

	return t;
}

void print( node* root, node* prev, int depth ) {

	if (depth == 0) std::cout << "tree: " << std::endl;

	for (int i = 0; i <= depth; i++) std::cout << "\t";
	if (root->bit <= prev->bit) std::cout << "backlink: ";
	std::cout << "bit: " << root->bit << " key: " << root->key << " value: " << root->value << " bc: " << root->bc << std::endl;

	if (root->bit > prev->bit) {
		print( root->c[0], root, depth+1 );
		print( root->c[1], root, depth+1 );
	}
}

int maxdepth = 5;

void bfs( node* root, node* prev, int depth ) {

	//if ( (root->bit <= prev->bit) && (root->bc == bc) ) {
	if (depth <= maxdepth) {
		std::cout << "\"";
		if (root->bit > prev->bit) std::cout << "i";
		std::cout << std::internal << std::hex << std::right << std::setw(4) << std::setfill('0');
		std::cout << (int16_t)(root->value) << "\"; ";

		std::cout << "\"";
		if (root->bit > prev->bit) std::cout << "i";
		std::cout << std::internal << std::hex << std::right << std::setw(4) << std::setfill('0') << (int16_t)(root->value) << "\" -> ";
		std::cout << "\"";
		if (root->c[0]->bit > root->bit) std::cout << "i";
		std::cout << std::internal << std::hex << std::right << std::setw(4) << std::setfill('0') << (int16_t)(root->c[0]->value) << "\"" << std::endl;

		std::cout << "\"";
		if (root->bit > prev->bit) std::cout << "i";
		std::cout << std::internal << std::hex << std::right << std::setw(4) << std::setfill('0') << (int16_t)(root->value) << "\" -> ";
		std::cout << "\"";
		if (root->c[1]->bit > root->bit) std::cout << "i";
		std::cout << std::internal << std::hex << std::right << std::setw(4) << std::setfill('0') << (int16_t)(root->c[1]->value) << "\"" << std::endl;
	}

	if (root->bit > prev->bit) {
		bfs( root->c[0], root, depth+1 );
		bfs( root->c[1], root, depth+1 );
	}
}


int main( int argc, const char* argv[] ) {

	dl_huffman_load_table( "tubecable_huffman.bin" );

	node* head = new node(); head->bit = -1;
	node* root = new node();

	dl_huffman_entry* table = &(dl_huffman_compact[DL_HUFFMAN_COUNT]);

	// build the tree
	for (int i = -DL_HUFFMAN_COUNT; i <= DL_HUFFMAN_COUNT; i++) {
		node* tmp = insert( root, table[i].seq, i );
		tmp->bc = table[i].size;
	}

	std::cout << "digraph huffmantree {\n ranksep=1\n nodesep=1\n overlap=false\n root=\"0000\"\n " << std::endl;

	bfs( root,head,0 );
	std::cout << "}" << std::endl;
}
