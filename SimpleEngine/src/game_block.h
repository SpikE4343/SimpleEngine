/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#ifndef PHYZAKS_GAME_BLOCK_H_INCLUDED
#define PHYZAKS_GAME_BLOCK_H_INCLUDED

#define BLOCK_REMOVE_AT( block, index, name ) block->name[index] = block->name[block->iTail];
#define BLOCK_MEMBER( t, n, block_size ) t n[block_size]
#define BLOCK_MEMBER_ARRAY( t, n, s, block_size ) t n[block_size][s]

#define MEMBER( t, n ) t n[BLOCK_SIZE]
#define MEMBER_ARRAY( t, n, s ) t n[BLOCK_SIZE][s]

#define MEMBER( t, n ) t n[BLOCK_SIZE]
#define MEMBER_ARRAY( t, n, s ) t n[BLOCK_SIZE][s]

#define REMOVE( block, index, name ) block->name[index] = block->name[block->iTail];

#endif
