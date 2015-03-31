#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//START OF ALPHABET OPS///////////////////////////////////////////////////////////
const int ALPHABET_SIZE = 256;
const int EMPTY_BITFILE_BUFFER_READ_ONLY = -1;
const int FULL_BITFILE_BUFFER_READ_ONLY = 7;
const int EMPTY_BITFILE_BUFFER_WRITE_ONLY = 7;
const int FULL_BITFILE_BUFFER_WRITE_ONLY = -1;

void alphabet_print(int * this)
{
  int i;
  for(i = 0; i < ALPHABET_SIZE; i++)
  {
    printf("%c: %d\n", i, this[i]);

  }
}

void alphabet_populate(int * this)
{
  int i;
  for(i = 0; i < ALPHABET_SIZE; i++)
  {
    this[i] = 1;
  }
}
//END OF ALPHABET OPS///////////////////////////////////////////////////////////


//START OF HUFFMAN OPS//////////////////////////////////////////////////////////
struct huffman_node
{
  union
  {
    struct
    {
      struct huffman_node * left;
      struct huffman_node * right;
    } branch;
    char nodeChar;
  } type;
  int isLeaf;
  int combinedOccurrence;
};

struct huffman_node * huffman_node_leaf(char c, int occurrence)
{
  struct huffman_node * result = malloc(sizeof(struct huffman_node));
  result->type.nodeChar = c;
  result->combinedOccurrence = occurrence;
  result->isLeaf = 1;
  return result;
}

//combinedOccurrence is derived from occurrence of left + occurence of right
struct huffman_node * huffman_node_branch(struct huffman_node * passedLeft, struct huffman_node * passedRight)
{
  struct huffman_node * result = malloc(sizeof(struct huffman_node));
  result->type.branch.left = passedLeft;
  result->type.branch.right = passedRight;
  result->isLeaf = 0;
  result->combinedOccurrence = passedLeft->combinedOccurrence + passedRight->combinedOccurrence;
  return result;
}

int huffman_tree_count_nodes(struct huffman_node ** tree)
{
  int result = 0;
  int i;
  for(i = 0; i < ALPHABET_SIZE; i++)
  {
    result += (tree[i] == NULL)? 0 : 1;
  }
  return result;
}

//assumes the tree is not null.
//deletes the least node from the tree and returns a pointer to it.
struct huffman_node * huffman_tree_delete_least_node(struct huffman_node ** tree)
{
  struct huffman_node * result = NULL;
  int i;
  int indexForDeletion = -1;
  for(i = 0; i < ALPHABET_SIZE; i++)
  {
    if(result == NULL)
    {
      result = tree[i];
      indexForDeletion = i;
    }
    else if(result != NULL && tree[i] != NULL)
    {
      if(result->combinedOccurrence > tree[i]->combinedOccurrence)
      {
        result = tree[i];
        indexForDeletion = i;
      }
    }
  }
  if(indexForDeletion != -1)
  {
    tree[indexForDeletion] = NULL;
  }
  return result;
}


//inserts the node into the least index, if possible.
void huffman_tree_insert_node(struct huffman_node ** tree, struct huffman_node * node)
{
  int i;
  int inserted = 0;
  for(i = 0; i < ALPHABET_SIZE && inserted == 0; i++)
  {
    if(tree[i] == NULL)
    {
      tree[i] = node;
      inserted = 1;
    }
  }
  return;
}

//assumes the tree has already been constructed.
struct huffman_node * huffman_tree_get_root(struct huffman_node ** tree)
{
  assert(huffman_tree_count_nodes(tree) == 1);
  struct huffman_node * result = NULL;
  int i;
  for(i = 0; i < ALPHABET_SIZE && result == NULL; i++)
  {
    if(tree[i] != NULL)
    {
      result = tree[i];
    }
  }
  return result;
}

struct huffman_node ** new_huffman_tree(int * alphabet)
{
  struct huffman_node ** result = malloc(sizeof(struct huffman_node) * ALPHABET_SIZE);
  int i;
  for(i = 0; i < ALPHABET_SIZE; i++)
  {
    result[i] = huffman_node_leaf(i, alphabet[i]);
  }
  while(huffman_tree_count_nodes(result) > 1)
  {
    struct huffman_node * firstNode = huffman_tree_delete_least_node(result);
    struct huffman_node * secondNode = huffman_tree_delete_least_node(result);
    assert(firstNode != NULL && secondNode != NULL);
    struct huffman_node * branch = huffman_node_branch(secondNode, firstNode);
    huffman_tree_insert_node(result, branch);
  }
  return result;
}

void huffman_tree_encoding_list_helper(struct huffman_node * root, int * path, int depth)
{
  if(!root->isLeaf)
  {
    path[depth] = 0;
    huffman_tree_encoding_list_helper(root->type.branch.left, path, depth + 1);
    path[depth] = 1;
    huffman_tree_encoding_list_helper(root->type.branch.right, path, depth + 1);
  }
  else
  {
    printf("%c", root->type.nodeChar);
    int i;
    for(i = 0; i < depth; i++)
    {
      printf("%d", path[i]);
    }
    printf("\n");
  }
}

void huffman_tree_encoding_list(struct huffman_node ** tree)
{
  struct huffman_node * root = huffman_tree_get_root(tree);
  int * path = malloc(sizeof(int) * ALPHABET_SIZE);
  huffman_tree_encoding_list_helper(root, path, 0);
}




//END OF HUFFMAN OPS//////////////////////////////////////////////////////////


//START OF BITFILE_OPS//////////////////////////////////////////////////////////

struct bitfile
{
	FILE * file;
	unsigned char buffer;
	int bufferIndex;
  int isReadOnly;
  int isWriteOnly;
};

struct bitfile * new_bitfile(char * filename, char * fOpenMode)
{
	struct bitfile * result;
	result = malloc(sizeof(struct bitfile));
	result->file = fopen(filename, fOpenMode);
	assert(result->file != NULL);
	result->buffer = 0;
	result->bufferIndex = -1;
  if(strcmp(fOpenMode, "r") == 0)
  {
    result->isReadOnly = 1;
    result->isWriteOnly = 0;
    result->bufferIndex = EMPTY_BITFILE_BUFFER_READ_ONLY;
  }
  else
  {
    assert(strcmp(fOpenMode, "w") == 0);
    result->isReadOnly = 0;
    result->isWriteOnly = 1;
    result->bufferIndex = EMPTY_BITFILE_BUFFER_WRITE_ONLY;
  }
	return result;
}


//returns next bit if present, else returns -1
int bitfile_read_only_next_bit(struct bitfile * this)
{
  assert(this->isReadOnly);
  if(!feof(this->file))
  {
  	if(this->bufferIndex == EMPTY_BITFILE_BUFFER_READ_ONLY)
  	//refill buffer
  	{
  		this->buffer = fgetc(this->file);
  		this->bufferIndex = FULL_BITFILE_BUFFER_READ_ONLY;
  	}
  	if(!feof(this->file))
  	{
  		unsigned char tempBuffer = this->buffer;
  		int result = tempBuffer >> this->bufferIndex;
  		this->bufferIndex = this->bufferIndex - 1;
  		return result & 1;
  	}
    else
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }
}

void bitfile_write_only_next_bit(struct bitfile * this, int bit)
{
  assert(this->isWriteOnly && (bit == 0 || bit == 1));
  int bitToBeWritten = bit << this->bufferIndex;
  this->buffer = this->buffer | bitToBeWritten;
  this->bufferIndex = this->bufferIndex - 1;
  if(this->bufferIndex == FULL_BITFILE_BUFFER_WRITE_ONLY)
  {
    fputc(this->buffer, this->file);
    this->buffer = 0;
    this->bufferIndex = EMPTY_BITFILE_BUFFER_WRITE_ONLY;
  }
}



//END OF BITFILE_OPS//////////////////////////////////////////////////////////

//START OF ENCODING OPS///////////////////////////////////////////////////////

void get_char_encoding_helper(struct huffman_node * root, int * path, int depth,
  char targetChar, int * scroll)
{
  if(!root->isLeaf)
  {
    path[depth] = 0;
    get_char_encoding_helper(root->type.branch.left, path, depth + 1, targetChar, scroll);
    path[depth] = 1;
    get_char_encoding_helper(root->type.branch.right, path, depth + 1, targetChar, scroll);
  }
  else if(root->type.nodeChar == targetChar)
  {
    int i;
    for(i = 0; i < depth; i++)
    {
      scroll[i] = path[i];
    }
  }
}

int * new_scroll()
{
  int * result = malloc(sizeof(int) * ALPHABET_SIZE);
  int i;
  for(i = 0; i < ALPHABET_SIZE; i++)
  {
    result[i] = -1;
  }
  return result;
}

int * get_char_encoding(struct huffman_node ** tree, char targetChar)
{
  int * result = new_scroll();
  struct huffman_node * root = huffman_tree_get_root(tree);
  int * path = malloc(sizeof(int) * ALPHABET_SIZE);
  get_char_encoding_helper(root, path, 0, targetChar, result);
  return result;
}

void encode(struct huffman_node ** tree, char* input, char * output)
{
  FILE * fileInput = fopen(input, "r");
  struct bitfile * fileOutput = new_bitfile(output, "w");
  char currentChar = fgetc(fileInput);
  while(!feof(fileInput))
  {
    int * currentCharEncoding = get_char_encoding(tree, currentChar);
    int i;
    for(i = 0; currentCharEncoding[i] != -1; i++)
    {
      bitfile_write_only_next_bit(fileOutput, currentCharEncoding[i]);
    }
    currentChar = fgetc(fileInput);
  }
}


//END OF ENCODING OPS///////////////////////////////////////////////////////

//START OF DECODING OPS///////////////////////////////////////////////////////

char get_char_decoding(struct huffman_node * root, struct bitfile * input)
{
  while(!root->isLeaf)
  {
    if(!bitfile_read_only_next_bit(input))
    {
      root = root->type.branch.left;
    }
    else
    {
      root = root->type.branch.right;
    }
  }
  return root->type.nodeChar;
}

void decode(struct huffman_node ** tree, char * input, char * output)
{
  FILE * inputFile = fopen(input, "r");
  struct bitfile * inputBitFile = new_bitfile(input, "r");
  FILE * outputFile = fopen(output, "w");
  struct huffman_node * root = huffman_tree_get_root(tree);
  while(!feof(inputBitFile->file))
  {
    char currentDecodedChar = get_char_decoding(root, inputBitFile);
    fputc(currentDecodedChar, outputFile);
  }

}

//END OF DECODING OPS///////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
  int * alphabet = malloc(sizeof(int) * ALPHABET_SIZE);
  alphabet_populate(alphabet);
  unsigned char c;
  FILE * file;
  if(argc != 5)
  {
    fprintf(stderr, "Useage: huffcode/huffdecode <filename> <filename> <filename>\n");
      exit(1);      // exit with error code
  }
  if(strcmp(argv[1], "huffcode") == 0)
  {
    file = fopen(argv[2], "r");
    assert( file != NULL );
    c = fgetc(file);
    while( !feof(file) ) 
    {
      c = fgetc(file);
      alphabet[c] = alphabet[c] + 1;
    }
    fclose(file);
    struct huffman_node ** encodeTree = new_huffman_tree(alphabet);
    encode(encodeTree, argv[3], argv[4]);
  }
  else if(strcmp(argv[1], "huffdecode") == 0)
  {
    file = fopen(argv[2], "r");
    assert( file != NULL );
    c = fgetc(file);
    while( !feof(file) ) 
    {
      c = fgetc(file);
      alphabet[c] = alphabet[c] + 1;
    }
    fclose(file);
    struct huffman_node ** encodeTree = new_huffman_tree(alphabet);
    decode(encodeTree, argv[3], argv[4]);
  }
  free(alphabet);
  return 0;
}


///////////////////////////////////////////////////\n