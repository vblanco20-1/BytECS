#include "BitTree.h"
#include <iostream>
#include <entt.hpp>

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"


TEST_CASE("Index Add values", "[bit-tree]") {

	ByteTree tree = create_bytetree();
	add_tree_val(&tree, 10, 10);
	add_tree_val(&tree, 100, 100);
	add_tree_val(&tree, 1000, 1000);
	add_tree_val(&tree, 10000, 10000);
	add_tree_val(&tree, 100000, 100000);
	add_tree_val(&tree, 1000000, 1000000);

	uint64_t val;
	REQUIRE(get_tree_val(&tree,10, val));
	REQUIRE(val == 10);

	REQUIRE(get_tree_val(&tree, 100, val));
	REQUIRE(val == 100);

	REQUIRE(get_tree_val(&tree, 1000, val));
	REQUIRE(val == 1000);

	REQUIRE(get_tree_val(&tree, 10000, val));
	REQUIRE(val == 10000);

	REQUIRE(get_tree_val(&tree, 100000, val));
	REQUIRE(val == 100000);

	REQUIRE(get_tree_val(&tree, 1000000, val));
	REQUIRE(val == 1000000);
}


TEST_CASE("Bitmask operations") {
	node_pool_index = 0;

	ByteNode* nodeA = allocate_treenode();

	//for (int i = 0; i < 256; i++) {
	set_node_mask_at(nodeA, 5);
	REQUIRE(get_node_mask_at(nodeA, 5));

	set_node_mask_at(nodeA, 0);
	REQUIRE(get_node_mask_at(nodeA, 0));

	
	REQUIRE(!get_node_mask_at(nodeA, 1));
	REQUIRE(!get_node_mask_at(nodeA, 254));
	set_node_mask_at(nodeA, 255);
	REQUIRE(get_node_mask_at(nodeA, 255));
		

	ByteNode* nodeB = allocate_treenode();
	for (int i = 0; i < 256; i++) {
		set_node_mask_at(nodeB, i);
	}

	uint64_t bitmaskA[4];
	for (int i = 0; i < 4; i++) {
		bitmaskA[i] = 0;
	}
	uint64_t bitmaskB[4];
	for (int i = 0; i < 4; i++) {
		bitmaskB[i] = uint64_t(-1);
	}
	uint64_t bitmaskC[4];

	uint64_t* bitmasks[] = { &bitmaskA[0],&bitmaskB[0] };

	merge_bitmasks(bitmasks, bitmaskC, 2, 4);

	auto count_bitmask = [](uint64_t* in_mask) ->int {
		int count = 0;
		bitmask_optimal_iterate(in_mask, 4, [&count](auto i) {
			count++;
			});
		return count;
	};

	REQUIRE(count_bitmask(bitmaskA) == 0);
	REQUIRE(count_bitmask(bitmaskB) == 256);
	REQUIRE(count_bitmask(bitmaskC) == 0);
	
	//}
	

	node_pool_index = 0;

}


int main(int argc, char* argv[])
{
	Catch::Session session; // There must be exactly one instance

	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0) // Indicates a command line error
		return returnCode;

	int numFailed = session.run();

	return numFailed;
}
