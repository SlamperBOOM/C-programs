#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <malloc.h>
#define countofsymb 256
#define byte 8

typedef struct Tree
{
    unsigned char symbol[countofsymb];
    struct Tree* left;
    struct Tree* right;
    int value;
    int lenghtofsymbol;
}Tree;

void CleanTree(Tree* tree)
{
    if (tree == NULL)
    {
        return;
    }
    else
    {
        CleanTree(tree->left);
        CleanTree(tree->right);
        free(tree);
    }
}

void Swap(Tree** array, int index1, int index2)
{
    Tree* a = array[index1];
    array[index1] = array[index2];
    array[index2] = a;
}

int CalcDepthOfTree(Tree* tree)
{
    if (tree == NULL)
    {
        return 0;
    }
    int depth1 = CalcDepthOfTree(tree->left), depth2 = CalcDepthOfTree(tree->right);
    if (depth1 > depth2)
    {
        return depth1 + 1;
    }
    else
    {
        return depth2 + 1;
    }
}

int ConvertFromCharToInt(unsigned char* code)
{
    int number = 0, N = 0;
    while (code[N] != '\0')
    {
        number *= 2;
        number += code[N] & 0x0F;
        N++;
    }
    return number;
}

void ConvertFromIntToChar(int symbol, unsigned char* code)
{
    for (int i = 7; i >= 0; i--)
    {
        code[i] = symbol % 2 | 0x30;
        symbol /= 2;
    }
    code[byte] = '\0';
}

void WriteByte(FILE* output, unsigned char* buffer)
{
    buffer[byte] = '\0';
    unsigned char outputsymbol = ConvertFromCharToInt(buffer);
    fwrite(&outputsymbol, sizeof(char), 1, output);
    for (int i = 0; i < byte + 1; i++)
    {
        buffer[i] = -1;
    }
}

void WriteBit(unsigned char* buffer, int* N, FILE* output, char bit)
{
    if (*(N) == 8)
    {
        WriteByte(output, buffer);
        *N = 0;
    }
    buffer[*N] = bit;
    *(N) += 1;
}

void ReadByte(FILE* input, unsigned char* buffer)
{
    unsigned char symbol;
    if (fread(&symbol, sizeof(unsigned char), 1, input) == 0)
    {
        return;
    }
    ConvertFromIntToChar((unsigned char)symbol, buffer);
}

char ReadBit(unsigned char* buffer, int* N, FILE* input)
{
    if (*(N) == 8)
    {
        ReadByte(input, buffer);
        *N = 0;
    }
    *(N) += 1;
    return buffer[*(N)-1];
}

Tree* BuildHuffmanTree(int* population)
{
    Tree* leaves[countofsymb];
    leaves[0] = NULL;
    int N = 0;
    for (int i = 0; i < countofsymb; i++)
    {
        if (population[i] > 0)
        {
            leaves[N] = (Tree*)malloc(sizeof(Tree));
            leaves[N]->value = population[i];
            leaves[N]->symbol[0] = (unsigned char)i;
            leaves[N]->lenghtofsymbol = 1;
            leaves[N]->left = NULL;
            leaves[N]->right = NULL;
            N++;
        }
    }
    while (N > 1)
    {
        int minimum = leaves[0]->value, minindex = 0;
        for (int i = 0; i < N; i++)
        {
            if (leaves[i]->value < minimum)
            {
                minindex = i;
                minimum = leaves[i]->value;
            }
        }
        Swap(leaves, minindex, N - 1);
        minimum = leaves[0]->value;
        minindex = 0;
        for (int i = 0; i < N - 1; i++)
        {
            if (leaves[i]->value < minimum)
            {
                minindex = i;
                minimum = leaves[i]->value;
            }
        }
        Swap(leaves, minindex, N - 2);
        Tree* node = (Tree*)malloc(sizeof(Tree));
        node->value = leaves[N - 1]->value + leaves[N - 2]->value;
        int a = 0;
        while (a < leaves[N - 1]->lenghtofsymbol)
        {
            node->symbol[a] = leaves[N - 1]->symbol[a];
            a++;
        }
        int b = 0;
        while (b < leaves[N - 2]->lenghtofsymbol)
        {
            node->symbol[a] = leaves[N - 2]->symbol[b];
            a++;
            b++;
        }
        node->lenghtofsymbol = leaves[N - 1]->lenghtofsymbol + leaves[N - 2]->lenghtofsymbol;
        node->right = leaves[N - 1];
        node->left = leaves[N - 2];
        leaves[N - 2] = node;
        N--;
    }
    return leaves[0];
}

Tree* RecoverHuffmanTree(FILE* input, int* N, unsigned char* buffer)
{
    Tree* codetree = (Tree*)malloc(sizeof(Tree));
    if (ReadBit(buffer, N, input) == '1')
    {
        codetree->left = RecoverHuffmanTree(input, N, buffer);
        codetree->right = RecoverHuffmanTree(input, N, buffer);
    }
    else
    {
        unsigned char symbol[9];
        for (int i = 0; i < byte; i++)
        {
            symbol[i] = ReadBit(buffer, N, input);
        }
        symbol[byte] = '\0';
        unsigned char convertedsymbol = ConvertFromCharToInt(symbol);
        codetree->symbol[0] = convertedsymbol;
        codetree->symbol[1] = '\0';
        codetree->left = NULL;
        codetree->right = NULL;
    }
    return codetree;
}

void PrintHuffmanTree(Tree* codetree, FILE* output, int* N, unsigned char* buffer)
{
    if (codetree == NULL)
    {
        return;
    }
    if (codetree->left != NULL)
    {
        WriteBit(buffer, N, output, '1');
        PrintHuffmanTree(codetree->left, output, N, buffer);
        PrintHuffmanTree(codetree->right, output, N, buffer);
    }
    else
    {
        WriteBit(buffer, N, output, '0');
        unsigned char symbol[9];
        ConvertFromIntToChar(codetree->symbol[0], symbol);
        for (int i = 0; i < byte; i++)
        {
            WriteBit(buffer, N, output, symbol[i]);
        }
    }
}

void Compress(FILE* input)
{
    //чтение текста
    unsigned char symbol;
    int lengthoftext = 0;
    int population[countofsymb];
    for (int i = 0; i < countofsymb; i++)
    {
        population[i] = 0;
    }
    while (fread(&symbol, sizeof(unsigned char), 1, input) != 0)
    {
        population[(unsigned char)symbol]++;
        lengthoftext++;
    }
    if (lengthoftext == 0)
    {
        fclose(input);
        return;
    }
    Tree* codetree = BuildHuffmanTree(population);
    int depthoftree = CalcDepthOfTree(codetree);
    //вывод сжатых данных
    FILE* output = fopen("out.txt", "wb");
    fwrite(&lengthoftext, sizeof(unsigned int), 1, output);
    int N = 0;
    unsigned char buffer[9];
    PrintHuffmanTree(codetree, output, &N, buffer);
    Tree* treebuffer = codetree;
    fclose(input);
    FILE *newinput = fopen("in.txt", "rb");
    char garbagesymbol[3];
    if (fread(&garbagesymbol, sizeof(char), 3, newinput) != 3)
    {
        fclose(output);
        fclose(newinput);
        return;
    }
    for (int i = 0; i < lengthoftext; i++)
    {
        if (fread(&symbol, sizeof(char), 1, newinput) != 1)
        {
            fclose(output);
            fclose(newinput);
            return;
        }
        for (int j = 0; j < depthoftree; j++)
        {
            if (treebuffer->left == NULL)
            {
                break;
            }
            int a = 0;
            while (a < treebuffer->left->lenghtofsymbol)
            {
                if (treebuffer->left->symbol[a] == symbol)
                {
                    WriteBit(buffer, &N, output, '0');
                    treebuffer = treebuffer->left;
                    a = -1;
                    break;
                }
                a++;
            }
            if (a != -1)
            {
                WriteBit(buffer, &N, output, '1');
                treebuffer = treebuffer->right;
            }
        }
        treebuffer = codetree;
    }
    if (N != 0)
    {
        for (int i = N; i < byte; i++)
        {
            buffer[i] = '0';
        }
        WriteByte(output, buffer);
    }
    fclose(output);
    fclose(newinput);
    CleanTree(codetree);
}

void Decompress(FILE* input)
{
    //чтение необходимых данных
    unsigned int lengthoftext;
    if (fread(&lengthoftext, sizeof(unsigned int), 1, input) != 1)
    {
        return;
    }
    int N = 0;
    unsigned char buffer[9];
    unsigned char symbol;
    if (fread(&symbol, sizeof(unsigned char), 1, input) == 0)
    {
        return;
    }
    ConvertFromIntToChar((unsigned char)symbol, buffer);
    Tree* codetree = RecoverHuffmanTree(input, &N, buffer);
    //воссоздание текста
    FILE* output = fopen("out.txt", "wb");
    Tree* treebuffer = codetree;
    while (1)
    {
        while (treebuffer->left != NULL)
        {
            char bit = ReadBit(buffer, &N, input);
            if (bit == '1')
            {
                treebuffer = treebuffer->right;
            }
            else
            {
                treebuffer = treebuffer->left;
            }
        }
        fwrite(&treebuffer->symbol[0], sizeof(char), 1, output);
        treebuffer = codetree;
        lengthoftext--;
        if (lengthoftext == 0)
        {
            break;
        }
    }
    fclose(output);
    fclose(input);
    CleanTree(codetree);
}

int main()
{
    char typeofprocess[3];
    FILE* input = fopen("in.txt", "rb");
    if (fread(&typeofprocess, sizeof(char), 3, input) != 3)
    {
        fclose(input);
        return 0;
    }
    if (typeofprocess[0] == 'c')
    {
        Compress(input);
    }
    else
    {
        Decompress(input);
    }
    return 0;
}
