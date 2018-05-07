#ifndef _HUFFMAN_TREE_
#define _HUFFMAN_TREE_

#include <stdlib.h>

typedef struct HuffmanTreeNode {
	unsigned char			symbol;
	unsigned char			isLeaf;
	struct HuffmanTreeNode	*child[2];
} HuffmanTreeNode;

void printSpace(int n) {
	for (int i=0; i<n; i++)
		printf("-");
}

void printHuffmanTree(HuffmanTreeNode *head, int depth) {

	if (head->isLeaf) {
		printSpace(depth);
		printf("0x%2X\n", head->symbol);
		return;
	}
	for (int i=0; i<2; i++) {
		printSpace(depth);
		printf("%d\n", i);
		if (head->child[i] == NULL) {
			return;
		} else {
			printHuffmanTree(head->child[i], depth+1);
		}
	}
}

HuffmanTreeNode *newHuffmanTreeNode() {

	HuffmanTreeNode *ptr
		= (HuffmanTreeNode *) malloc(sizeof(HuffmanTreeNode));
	if (ptr) {
		ptr->isLeaf = 0;
		//ptr->symbol = someImpossibleValue();
		ptr->child[0] = NULL;
		ptr->child[1] = NULL;
	} else errorMsg("ptr = NULL");
	return ptr;
}

HuffmanTreeNode *newHuffmanTreeLeaf(unsigned char symbol) {

	HuffmanTreeNode *ptr
		= (HuffmanTreeNode *) malloc(sizeof(HuffmanTreeNode));
	if (ptr) {
		ptr->symbol = symbol;
		ptr->isLeaf = 1;
		ptr->child[0] = NULL;
		ptr->child[1] = NULL;
	}
	return ptr;
}

HuffmanTreeNode *newHuffmanTreeLeaves(
					int numberOfLeaves, unsigned char symbols[]) {
	HuffmanTreeNode *ptr;
	switch (numberOfLeaves) {
		case 0:
			return NULL;
		case 1:
			return newHuffmanTreeLeaf(symbols[0]);
		default:
			ptr = (HuffmanTreeNode *)malloc(
						sizeof(HuffmanTreeNode) * numberOfLeaves );
			if (ptr) {
				for (int i=0; i<numberOfLeaves; i++) {
					ptr[i].symbol = symbols[i];
					ptr[i].isLeaf = 1;
					ptr[i].child[0] = NULL;
					ptr[i].child[1] = NULL;
				}
			}
			return ptr;
	}
}

//recursively
//return: number of leaves successfully put under a node
int putLeavesUnderHuffmanTreeNode (
		HuffmanTreeNode *node, HuffmanTreeNode *leaves
		, unsigned char numberOfLeaves, unsigned char depth) {

	if (node->isLeaf || numberOfLeaves == 0)
		return 0;

	unsigned char leavesPutCount = 0, numberOfLeavesPut;

	switch (depth) {
		case 0:	//does such case exsist?
			break;
		case 1:	//terminal condition
			for (int i=0; i<2 && leavesPutCount < numberOfLeaves; i++)
				if (node->child[i] == NULL) {
					node->child[i] = &leaves[leavesPutCount];
					leavesPutCount++;
				}
			break;
		default://recurses
			for (int i=0; i<2 && leavesPutCount < numberOfLeaves; i++) {
				if (node->child[i] == NULL)
					node->child[i] = newHuffmanTreeNode();
				numberOfLeavesPut
					= putLeavesUnderHuffmanTreeNode(
							node->child[i]
							, leaves + leavesPutCount
							, numberOfLeaves - leavesPutCount
							, depth-1
						);
				leavesPutCount += numberOfLeavesPut;
			}
			break;
	}
	return leavesPutCount;
}

#endif