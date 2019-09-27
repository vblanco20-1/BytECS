

#include <cstdint>
#include <cstring>
#include <assert.h>
#include <vector>
#define _PERF_TRYHARD


struct ByteNode
{
	uint64_t bytemask[4];
	union
	{
		uint64_t vals[256];
		ByteNode* childs[256];
	};
};



template<typename F>
void bitmask_optimal_iterate(const uint64_t* bitmask, int count, F&& function)
{
#ifndef _PERF_TRYHARD
	for (int idx = 0; idx < count; idx++)
	{
		const int begin = idx * 64;
		if (bitmask[idx] != 0)
		{
			for (int i = 0; i < 64; i++)
			{
				uint64_t mask = (uint64_t(0x1) << i);

				if (bitmask[idx] & mask)
				{
					function(begin + i);
				}
			}
		}
	}

#else
	uint8_t indices[64];
	uint8_t nindices;
	
	for (int idx = 0; idx < count; idx++)
	{		
		if (bitmask[idx] != 0)
		{
			const int begin = idx * 64;
			nindices = 0;
			for (int i = 0; i < 64; i++)
			{
				uint64_t mask = (uint64_t(0x1) << i);

				if (bitmask[idx] & mask)
				{
					indices[nindices++] = i;
				}
			}
			
			{
				for(int i = 0 ; i < nindices ; i++)
				{
					int index = begin + indices[i];
					
					function(index);
				}
			}
		}
	}
#endif

}
 bool  get_node_mask_at(ByteNode* node, const uint8_t index)
{
	const uint8_t idx = (index >> 6);
	const uint8_t shift = (index & 0x3F);
	const uint64_t mask = (uint64_t(0x1) << shift);
	const uint64_t andmask = node->bytemask[idx] & mask;
	return andmask;
}
 void set_node_mask_at(ByteNode* node, const uint8_t index)
{
	const uint8_t idx = (index >> 6);
	const uint8_t shift = (index & 0x3F);
	const uint64_t mask = (uint64_t(0x1) << shift);
	node->bytemask[idx] |= mask;
}
 void clear_node_mask_at(ByteNode* node, const uint8_t index)
{
	const uint8_t idx = (index >> 6);
	const uint8_t shift = (index & 0x3F);
	const uint64_t mask = (uint64_t(0x1) << shift);
	const uint64_t invmask = uint64_t(-1) ^ mask;
	node->bytemask[idx] &= invmask;
}

__inline bool is_node_empty(ByteNode* node)
{
	return !(node->bytemask[0] | node->bytemask[1] | node->bytemask[2] | node->bytemask[3]);
}

struct ByteTree
{
	ByteNode* root;

	uint32_t capacity;
	char depth;
};



static ByteNode node_pool[256 * 256 * 10];
int node_pool_index = 0;

int allocations = 0;
ByteNode* allocate_treenode()
{
	allocations++;
	ByteNode* node = &node_pool[node_pool_index++];//new ByteNode();

	memset(node, 0, sizeof(ByteNode));

	return node;
}

int deletions = 0;
void free_treenode(ByteNode* node)
{
	deletions++;
	//delete node;
	//delete node;
}

ByteTree create_bytetree()
{
	ByteTree tree;
	tree.capacity = 256 ^ 3;
	tree.root = allocate_treenode();
	tree.depth = 3;
	return tree;
}

void grow_tree(ByteTree* tree, uint32_t new_capacity)
{
	int level = 1;
	if (new_capacity >= 256 && new_capacity < 256 * 256)
	{
		level = 2;

	}
	else if (new_capacity >= 256 * 256 && new_capacity < 256 * 256 * 256)
	{
		level = 3;
	}

	while (tree->depth < level)
	{
		ByteNode* child = tree->root;
		ByteNode* newroot = allocate_treenode();
		newroot->childs[0] = child;
		set_node_mask_at(newroot, 0);
		tree->root = newroot;
		tree->capacity = 256 ^ level;
		tree->depth++;
	}
}


void add_tree_val(ByteTree* tree, uint32_t index, uint64_t value)
{
	if (index >= tree->capacity)
	{
		grow_tree(tree, index);
	}

	int level = tree->depth;
	ByteNode* node = tree->root;

	while (true)
	{
		int shift = 8 * (level - 1);
		uint32_t shifted = (index >> shift) & 0xFF;

		if (get_node_mask_at(node, shifted))
		{
			if (level == 1)
			{
				node->vals[shifted] = value;
				return;
			}
			else
			{
				level--;
				node = node->childs[shifted];
			}
		}
		else
		{
			set_node_mask_at(node, shifted);
			if (level == 1)
			{
				node->vals[shifted] = value;
				return;
			}
			else
			{
				level--;
				node->childs[shifted] = allocate_treenode();
				node = node->childs[shifted];
			}
		}
	}
}

bool get_tree_val(const ByteTree* tree, uint32_t index, uint64_t& value)
{
	int level = tree->depth;
	ByteNode* node = tree->root;

	while (true)
	{
		int shift = 8 * (level - 1);
		uint32_t shifted = (index >> shift) & 0xFF;

		if (get_node_mask_at(node, shifted))
		{
			if (level == 1)
			{
				value = node->vals[shifted];
				return true;
			}
			else
			{
				level--;
				node = node->childs[shifted];
			}
		}
		else
		{
			return false;
		}
	}
}

bool remove_tree_recursive(ByteNode* node, uint32_t index, int level, bool& clear_parent)
{
	int shift = 8 * (level - 1);
	uint32_t shifted = (index >> shift) & 0xFF;

	if (level == 1)
	{
		clear_node_mask_at(node, shifted);

		if (is_node_empty(node))
		{
			clear_parent = true;
		}

		return true;
	}
	else if (get_node_mask_at(node, shifted))
	{
		bool bclear = false;

		bool bfound = remove_tree_recursive(node->childs[shifted], index, level - 1, bclear);

		if (bclear)
		{
			free_treenode(node->childs[shifted]);
			clear_node_mask_at(node, shifted);

			if (is_node_empty(node))
			{
				clear_parent = true;
			}
		}
		return bfound;
	}
	else
	{
		return false;
	}
}
#include <immintrin.h>
__inline void merge_bitmasks(uint64_t** bitmasks, uint64_t* out, uint8_t nbitmasks, uint8_t count)
{
#if 1
	for (int i = 0; i < count; i++)
	{
		out[i] = bitmasks[0][i];
	}

	for (int m = 1; m < nbitmasks; m++)
	{
		for (int i = 0; i < count; i++)
		{
			out[i] &= bitmasks[m][i];
		}
	}
#else
	__m256i accum = _mm256_load_si256((__m256i*)bitmasks[0]);

	for (int m = 1; m < nbitmasks; m++)
	{
		__m256i accum2 = _mm256_load_si256((__m256i*)bitmasks[m]);

		accum = _mm256_and_si256(accum, accum2);
	}
	//*(__m256i*(out)) = accum;
	_mm256_store_si256((__m256i*)out, accum);
#endif
}


template<typename F>
void iterate_joined_recursive(ByteNode** nodes, int nodecount, int depth, F function)
{
	uint64_t* bitmasks[10];

	for (int i = 0; i < nodecount; i++)
	{
		bitmasks[i] = &nodes[i]->bytemask[0];
	}
	uint64_t out_bitmask[4];
	merge_bitmasks(&bitmasks[0], &out_bitmask[0], nodecount, 4);

	if (depth == 1)
	{
		//uint64_t* bitmasks[10];
		//
		//for(int i = 0; i < nodecount; i++)
		//{
		//	bitmasks[i] = &nodes[i]->bytemask[0];
		//}
		//uint64_t out_bitmask[4];
		//merge_bitmasks(&bitmasks[0], &out_bitmask[0], nodecount, 4);
		//
		//int count = 4;
		//for(int idx = 0; idx < count; idx++)
		//{
		//	const int begin = idx * 64;
		//	if(out_bitmask[idx] != 0)
		//	{
		//		for(int i = 0; i < 64; i++)
		//		{
		//			uint64_t mask = (uint64_t(0x1) << i);
		//
		//			if(out_bitmask[idx] & mask)
		//			{
		//				//function();
		//				function(begin + i, nodes);
		//			}
		//		}
		//	}
		//}
#ifndef _PERF_TRYHARD
		bitmask_optimal_iterate(&out_bitmask[0], 4, [&function, &nodes](uint32_t index) {

			function(index, nodes);
			});
#endif

		
	}
	else
	{


		ByteNode* othernodes[10];

		bitmask_optimal_iterate(&out_bitmask[0], 4, [&](uint32_t index) {

			for (int i = 0; i < nodecount; i++)
			{
				othernodes[i] = nodes[i]->childs[index];
			}
#ifdef _PERF_TRYHARD
			if(depth == 2)
			{
				function(0, othernodes);
			}
			else
			{
				iterate_joined_recursive(othernodes, nodecount, depth - 1, function);
			}

#else 

			iterate_joined_recursive(othernodes, nodecount, depth - 1, function);
#endif
			});
	}
}

template<typename F>
void iterate_joined_trees(ByteTree* trees, int treecount, F&& function)
{

	ByteNode* rootnodes[10];

	for (int i = 0; i < treecount; i++)
	{
		rootnodes[i] = trees[i].root;
	}
#ifndef _PERF_TRYHARD
	iterate_joined_recursive(&rootnodes[0], treecount, 3, function);
#else
	iterate_joined_recursive(&rootnodes[0], treecount, 3, [&](uint32_t index, ByteNode** nodes) {
		function(index, nodes);
	});
#endif
}


template<typename F>
void iterate_tree_values(ByteTree* tree, F&& function)
{
	ByteNode* node_stack[4];
	int iteration_stack[4];
	int stack_head = 0;

	node_stack[0] = tree->root;
	iteration_stack[0] = -1;
	int current_depth = tree->depth;

	while (stack_head >= 0)
	{
	start:
		ByteNode* nd = node_stack[stack_head];
		int& iterator = iteration_stack[stack_head];

		if (current_depth == 1)
		{
			uint32_t begin_index = (iteration_stack[0] << 16) + (iteration_stack[1] << 8);

			for (int i = 0; i < 256; i++)
			{
				if (get_node_mask_at(nd, i))
				{
					function(begin_index + i, nd->vals[i]);
				}
			}

			//pop
			stack_head--;
			current_depth++;
		}
		else
		{
			while (iterator < 255)
			{
				iterator++;

				if (get_node_mask_at(nd, iterator))
				{
					//push
					stack_head++;
					current_depth--;
					node_stack[stack_head] = nd->childs[iterator];
					iteration_stack[stack_head] = -1;
					goto start;
				}
			}
			//pop
			stack_head--;
			current_depth++;
		}
	}
	return;
}

bool remove_tree_val(const ByteTree* tree, uint32_t index)
{
	int level = tree->depth;
	ByteNode* node = tree->root;

	bool bclear = false;
	return remove_tree_recursive(node, index, level, bclear);
}


template<typename T>
struct ComponentPool
{
	ByteTree tree;
	std::vector<T> Dense;
	std::vector<uint32_t> Reverse;
};
template<typename T>
void remove_pool_element(ComponentPool<T>* pool, uint32_t entity)
{
	uint32_t index = entity & 0xFFFFF;

	uint64_t val;
	if (get_tree_val(&pool->tree, index, val))
	{
		if (pool->Reverse[val] == entity)
		{
			auto dense_end = pool->Dense.size() - 1;
			auto reverse_end = pool->Reverse.size() - 1;

			//swap with last
			uint32_t swap_et = pool->Reverse[reverse_end];
			pool->Reverse[val] = pool->Reverse[reverse_end];
			pool->Dense[val] = pool->Dense[dense_end];

			pool->Dense.pop_back();
			pool->Reverse.pop_back();

			uint32_t swap_index = swap_et & 0xFFFFF;

			add_tree_val(&pool->tree, swap_index, val);
		}
	}
}
template<typename T>
bool get_pool_element(ComponentPool<T>* pool, uint32_t entity, T& value)
{
	uint32_t index = entity & 0xFFFFF;

	uint64_t val;
	if (get_tree_val(&pool->tree, index, val))
	{
		if (pool->Reverse[val] == entity)
		{
			value = pool->Dense[val];
			return true;
		}
	}
	else
	{
		return false;
	}
}
template<typename T>
__inline T& get_pool_element_raw(ComponentPool<T>* pool, uint32_t index)
{
	uint64_t val;
	bool found = get_tree_val(&pool->tree, index, val);
	assert(found);

	return pool->Dense[val];
}

template<typename T>
__inline uint32_t& get_pool_entity_from_index(ComponentPool<T>* pool, uint32_t index)
{
	uint64_t val;
	bool found = get_tree_val(&pool->tree, index, val);
	assert(found);

	return pool->Reverse[val];
}


template<typename T>
void add_pool_element(ComponentPool<T>* pool, uint32_t entity, const T& value)
{
	uint32_t index = entity & 0xFFFFF;

	uint64_t val;
	//already in set, replace
	if (get_tree_val(&pool->tree, index, val))
	{
		if constexpr  (sizeof(T) > 0)
		{
			if (pool->Reverse[val] == entity)
			{
				pool->Dense[val] = value;
			}
		}		
	}
	else
	{
		pool->Reverse.push_back(entity);
		if constexpr  (sizeof(T) > 0)
		{
			pool->Dense.push_back(value);
		}		

		auto dense_end = pool->Dense.size() - 1;
		add_tree_val(&pool->tree, index, dense_end);
	}
}

template<typename A, typename B, typename F>
void join_pools(ComponentPool<A>* poolA, ComponentPool<B>* poolB, F&& function)
{

	ByteTree trees[] = { poolA->tree, poolB->tree };

	iterate_joined_trees(trees, 2, [&](uint32_t id, ByteNode** nodes) {

#ifndef _PERF_TRYHARD
		auto eid = poolA->Reverse[nodes[0]->vals[id]];
		A& cA = poolA->Dense[nodes[0]->vals[id]];
		B& cB = poolB->Dense[nodes[1]->vals[id]];

		function(eid, cA, cB);
#else
		uint64_t* bitmasks[2] = { &nodes[0]->bytemask[0] ,&nodes[1]->bytemask[0] };

		uint64_t out_bitmask[4];
		merge_bitmasks(&bitmasks[0], &out_bitmask[0], 2, 4);

		uint8_t indices[64];
		uint8_t nindices;

		int count = 4;
		for(int idx = 0; idx < count; idx++)
		{
			const int begin = idx * 64;
			if(out_bitmask[idx] != 0)
			{
				nindices = 0;
				for (int i = 0; i < 64; i++)
				{
					uint64_t mask = (uint64_t(0x1) << i);

					if (out_bitmask[idx] & mask)
					{
						indices[nindices++] = i;
					}
				}
				{
					constexpr int prefetch = 0;
					if (prefetch > 0) {

						int j = 0;
						for (j; j < nindices - prefetch; j++) {

							int index = begin + indices[j];
							int nxindex = begin + indices[j + 3];

							_mm_prefetch((char*)& poolA->Reverse[nodes[0]->vals[nxindex]], 0);
							_mm_prefetch((char*)& poolA->Dense[nodes[0]->vals[nxindex]], 0);
							_mm_prefetch((char*)& poolB->Dense[nodes[0]->vals[nxindex]], 0);

							auto eid = poolA->Reverse[nodes[0]->vals[index]];
							A& cA = poolA->Dense[nodes[0]->vals[index]];
							B& cB = poolB->Dense[nodes[1]->vals[index]];

							function(eid, cA, cB);
						}
						for ( j ; j < nindices - 3; j++) {
							int index = begin + indices[j];

							auto eid = poolA->Reverse[nodes[0]->vals[index]];
							A& cA = poolA->Dense[nodes[0]->vals[index]];
							B& cB = poolB->Dense[nodes[1]->vals[index]];

							function(eid, cA, cB);
						}
					}
					else {
						for (int j = 0; j < nindices; j++) {
							int index = begin + indices[j];

							auto eid = poolA->Reverse[nodes[0]->vals[index]];
							A& cA = poolA->Dense[nodes[0]->vals[index]];
							B& cB = poolB->Dense[nodes[1]->vals[index]];

							function(eid, cA, cB);
						}
					}
				}
			}
		}

#endif
		});
}
template<typename A, typename B, typename C,typename F>
void join_pools(ComponentPool<A>* poolA, ComponentPool<B>* poolB, ComponentPool<C>* poolC, F&& function)
{

	ByteTree trees[] = { poolA->tree, poolB->tree ,poolC->tree};

	iterate_joined_trees(trees, 3, [&](uint32_t id, ByteNode** nodes) {

#ifndef _PERF_TRYHARD
		auto eid = poolA->Reverse[nodes[0]->vals[id]];
		A& cA = poolA->Dense[nodes[0]->vals[id]];
		B& cB = poolB->Dense[nodes[1]->vals[id]];
		C& cC = poolC->Dense[nodes[2]->vals[id]];
		function(eid, cA, cB,cC);
#else
		uint64_t* bitmasks[3] = { &nodes[0]->bytemask[0] ,&nodes[1]->bytemask[0],&nodes[2]->bytemask[0] };

		uint64_t out_bitmask[4];
		merge_bitmasks(&bitmasks[0], &out_bitmask[0], 3, 4);

		uint8_t indices[64];
		uint8_t nindices;

		int count = 4;
		for (int idx = 0; idx < count; idx++)
		{
			const int begin = idx * 64;
			if (out_bitmask[idx] != 0)
			{
				nindices = 0;
				for (int i = 0; i < 64; i++)
				{
					uint64_t mask = (uint64_t(0x1) << i);

					if (out_bitmask[idx] & mask)
					{
						indices[nindices++] = i;
					}
				}
				{
					constexpr int prefetch = 0;
					if (prefetch > 0) {

						int j = 0;
						for (j; j < nindices - prefetch; j++) {
					
							const int index = begin + indices[j];
							const int nxindex = begin + indices[j + prefetch];							
							
							const auto iA = nodes[0]->vals[index];
							const auto iB = nodes[1]->vals[index];
							const auto iC = nodes[2]->vals[index];

							auto eid = poolA->Reverse[iA];

							A& cA = poolA->Dense[iA];
							
							B& cB = poolB->Dense[iB];
							
							C& cC = poolC->Dense[iC];	

							const auto iAp = nodes[0]->vals[nxindex];
							const auto iBp = nodes[1]->vals[nxindex];
							const auto iCp = nodes[2]->vals[nxindex];

							_mm_prefetch((char*)&poolA->Reverse[iAp], 0);
							_mm_prefetch((char*)&poolA->Dense[iAp], 0);
							_mm_prefetch((char*)&poolB->Dense[iBp], 0);
							_mm_prefetch((char*)&poolC->Dense[iCp], 0);

							function(eid, cA, cB, cC);
						}
						for (j; j < nindices - prefetch; j++) {
							int index = begin + indices[j];
					
							auto eid = poolA->Reverse[nodes[0]->vals[index]];
							A& cA = poolA->Dense[nodes[0]->vals[index]];
							B& cB = poolB->Dense[nodes[1]->vals[index]];
							C& cC = poolC->Dense[nodes[2]->vals[index]];

							function(eid, cA, cB, cC);
						}
					}
					else 
					{
						for (int j = 0; j < nindices; j++) {
							int index = begin + indices[j];

							auto eid = poolA->Reverse[nodes[0]->vals[index]];
							A& cA = poolA->Dense[nodes[0]->vals[index]];
							B& cB = poolB->Dense[nodes[1]->vals[index]];
							C& cC = poolC->Dense[nodes[2]->vals[index]];

							function(eid, cA, cB, cC);
						}
					}			
				}
			}
		}

#endif
	});
}

template<typename T>
ComponentPool<T> create_pool()
{
	ComponentPool<T> pool;
	pool.tree = create_bytetree();
	return pool;
}

