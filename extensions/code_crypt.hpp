#ifndef CODE_CRYPT_HPP
#define CODE_CRYPT_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>
#include <ctime>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "core.hpp"

#define KEYLEN 32
#define STRLEN 1024

using namespace std;

// 超类
// class Export
// {
//     public:
//     virtual int exportContent(int src, int dest, const char * key) = 0;
//     virtual ~Export() = default;
// };

// leaf node of huffman tree
struct Node
{
    char key;
    int weight;
    Node *left;
    Node *right;
};

// format of HuffmanCode
struct huffmancode
{
    char ch;           // Huffman码的符号
    unsigned int code; // 对应的Huffman码
    unsigned char len; //Huffman码的长度
};

// 比较器
bool NodeCmp(Node *, Node *);

// 哈夫曼树
class HuffmanTree
{
public:
    Node *root;                    // 哈夫曼树根节点
    vector<huffmancode> code_list; // 哈夫曼码表

    // 构建哈夫曼树
    explicit HuffmanTree(map<char, int> m); // 传入map，存储字符及其频次

    ~HuffmanTree();

private:
    // 拷贝节点
    void NodeCopy(Node *des, Node *src);

    // 递归获取当前节点哈夫曼码
    void getCode(vector<huffmancode> *code_list, unsigned int *code, unsigned char len, Node *node);

    // 获取哈夫曼码
    void getHuffmanCode(vector<huffmancode> *code_list, Node *root);

    // 删除哈夫曼树，回收内存
    void deleteTree(Node *node);
};

// 压缩类
class ExportCompress : public Export
{
public:
    int exportContent(int r, int w, const char* keyin);
};

// 解压类
class ExportDecompress : public Export
{
public:
    int exportContent(int r, int w, const char* keyin);
};

// 加密类
class ExportEncrypt : public Export
{
public:
    int exportContent(int r, int w, const char *keyin);
};

// 解密类
class ExportDecrypt : public Export
{
public:
    int exportContent(int r, int w, const char *keyin);
};

// encode & encrypt
class ExportEncodeEncrypt : public Export
{
public:
    int exportContent(int src, int tgt, const char *passwd = nullptr);
};

// decode & decrypt (decrypt first)
class ExportDecodeDecrypt : public Export
{
public:
    int exportContent(int src, int tgt, const char *passwd = nullptr);
};

#endif // CODE_CRYPT_HPP