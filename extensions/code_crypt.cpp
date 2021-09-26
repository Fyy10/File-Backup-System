#include "code_crypt.hpp"

using namespace std;

// 比较器
bool NodeCmp(Node *n1, Node *n2)
{
    return n1->weight < n2->weight; // 默认从小到大排序
}

// Huffman Tree
HuffmanTree::HuffmanTree(map<char, int> m)
{
    vector<Node *> weight_order;
    // 遍历map，将节点传入vector，便于排序
    for (auto iter : m)
    {
        auto tmp = new Node;
        tmp->key = iter.first;
        tmp->weight = iter.second;
        tmp->left = nullptr;
        tmp->right = nullptr;
        weight_order.push_back(tmp);
    }

    // 只有一个节点
    if (weight_order.size() == 1)
    {
        auto father = new Node;
        father->weight = weight_order[0]->weight;

        // auto child = new Node;
        // NodeCopy(child, weight_order[0]);

        father->left = weight_order[0]; // child;
        father->right = nullptr;

        // delete weight_order[0];                     // 回收内存
        weight_order.erase(weight_order.begin()); // 删除指针

        weight_order.push_back(father);
    }
    else // 多于一个节点
    {
        while (weight_order.size() != 1)
        {
            sort(weight_order.begin(), weight_order.end(), NodeCmp);

            auto branch = new Node;
            branch->key = 0;
            branch->left = nullptr;
            branch->right = nullptr;
            branch->weight = weight_order[0]->weight + weight_order[1]->weight;

            // auto left_leaf = new Node;
            // auto right_leaf = new Node;
            // NodeCopy(left_leaf, weight_order[0]);
            // NodeCopy(right_leaf, weight_order[1]);

            branch->left = weight_order[0];  // left_leaf;
            branch->right = weight_order[1]; // right_leaf;

            //delete weight_order[0], weight_order[1];                            // 回收内存
            weight_order.erase(weight_order.begin(), weight_order.begin() + 2); // 删除指针

            weight_order.push_back(branch);
        }
    }
    this->root = weight_order[0]; //获得哈夫曼树的根节点
    weight_order.clear();

    getHuffmanCode(&(this->code_list), this->root);
}

HuffmanTree::~HuffmanTree()
{
    deleteTree(root);
}

// 拷贝节点
void HuffmanTree::NodeCopy(Node *des, Node *src)
{
    des->key = src->key;
    des->weight = src->weight;
    des->left = src->left;
    des->right = src->right;
}

// 递归获取当前节点哈夫曼码
void HuffmanTree::getCode(vector<huffmancode> *code_list, unsigned int *code, unsigned char len, Node *node)
{
    if (node->left == nullptr && node->right == nullptr) //该节点为叶子节点，存入哈夫曼码
    {
        huffmancode leaf = {
            .ch = node->key,
            .code = *code,
            .len = len};
        code_list->push_back(leaf);
        *code >>= 1; // 返回时删除最后的数字
        return;
    }
    if (node->left) // 若左子树存在
    {
        *code = (*code << 1) + 0;
        getCode(code_list, code, len + 1, node->left);
    }
    if (node->right) // 若右子树存在
    {
        *code = (*code << 1) + 1;
        getCode(code_list, code, len + 1, node->right);
    }
    *code >>= 1; // 返回时删除最后的数字

    return;
}

// 获取哈夫曼码
void HuffmanTree::getHuffmanCode(vector<huffmancode> *code_list, Node *root)
{
    unsigned int code = 0;
    getCode(code_list, &code, 0, root);
}

// 删除哈夫曼树，回收内存
void HuffmanTree::deleteTree(Node *node)
{
    if (node->left)
        deleteTree(node->left);
    if (node->right)
        deleteTree(node->right);
    delete node;
}

int ExportCompress::exportContent(int r, int w, const char* keyin)
{
    FILE *src, *des;
    src = fdopen(r, "r");
    des = fdopen(w, "w");

    char ch = 0;
    ch = getc(src);
    if(ch == EOF){
        // cerr << "Error: the file can not be empty!" << endl;
        // exit(1);
        fclose(src);
        fclose(des);
        return 0;
    }
    fseek(src, 0, SEEK_SET);

    map<char, int> m;
    char key = 0;

    //构建map
    if (src < 0)
        return 1;
    while (true)
    {
        if (fread(&key, sizeof(char), 1, src) == 0)
            break;
        //src.read(&key, 1);
        m[key] += 1;
        //if(src.peek() == EOF) break;
    }
    //构建哈夫曼树
    HuffmanTree huffmantree(m);

    string s;
    // 将哈夫曼表存入目的文件
    for(auto iter: m)
    {
        fwrite(&(iter.first), sizeof(char), 1, des);
        ch = '~';
        fwrite(&ch, sizeof(char), 1, des);
        string s = to_string(iter.second);
        fwrite(s.c_str(), sizeof(char), s.length(), des);
        ch = ' ';
        fwrite(&ch, sizeof(char), 1, des);
        // des << iter.first << "~" << iter.second << " ";
    }
    fwrite("!@#$%^&*()_+", sizeof(char), 12, des);
    // des << "!@#$%^&*()_+";  // haffman表结束标记

    fseek(src, 0, SEEK_SET);
    // src.seekg(0, ios::beg); // 文件指针返回文件头
    // 将源文件字符转为01串
    pair<unsigned int, unsigned char> remain(0, 0); // pair<code, len>
    unsigned char mask = 0b11111111;
    char tmp = 0;
    while (true)
    {
        if (fread(&key, sizeof(char), 1, src) == 0)
            break;
        // src.read(&key, 1);
        for (auto iter : huffmantree.code_list)
        {
            if (key == iter.ch) // 将当前符号转为哈夫曼码
            {
                remain.first = (remain.first << iter.len) + iter.code;
                remain.second += iter.len;
                while (remain.second >= 8)
                {
                    remain.second -= 8;
                    tmp = (char)(remain.first >> remain.second);
                    fwrite(&tmp, sizeof(char), 1, des);
                    // des << static_cast<char>(remain.first >> remain.second);
                    remain.first &= ~(mask << remain.second);
                }
                break;
            }
        }

        // if(src.peek() == EOF) break;
    }
    // 结尾长度不足八位，用0补齐
    if (remain.second != 0)
    {
        fwrite((char *)(&remain.first), sizeof(char), 1, des);
        // des << static_cast<char>(remain.first);
    }
    fwrite((char *)(&remain.second), sizeof(char), 1, des);
    // des << static_cast<char>(remain.second);

    fclose(src);
    fclose(des);

    return 0;
}

int ExportDecompress::exportContent(int r, int w, const char* keyin)
{
    FILE *src, *des;
    src = fdopen(r, "r");
    des = fdopen(w, "w");

    bitset<8> bits;
    int loc = 0;
    char ch = 0;
    int freq = 0;
    char tmp[11] = {0};
    string str;
    string endflag = "@#$%^&*()_+";
    string mapline;
    string filestream;
    map<char, int> m;

    ch = getc(src);
    if(ch == EOF){
        // cerr << "Error: the file can not be empty!" << endl;
        // exit(1);
        fclose(src);
        fclose(des);
        return 0;
    }
    fseek(src, 0, SEEK_SET);

    //读取map<字符，词频>
    while (true)
    {
        if (fread(&ch, sizeof(char), 1, src) == 0)
            break;
        // src.read(&ch, 1);
        if (ch == '!')
        {
            fread(tmp, sizeof(char), 11, src);
            // src.read(tmp, 11);
            if (string(tmp) == endflag)
                break;
            else
                fseek(src, -11, SEEK_CUR);
            // else src.seekg(-11, ios::cur);
        }
        mapline += ch;
        // if(src.peek() == EOF) break;
    }

    //还原map并还原huffman树
    while (!mapline.empty())
    {
        ch = mapline[0];
        mapline.erase(0, 2);
        loc = mapline.find(' ');
        str = mapline.substr(0, loc);
        mapline.erase(0, loc + 1);
        freq = stoi(str, nullptr, 10);
        m.insert(pair<char, int>(ch, freq));
    }
    HuffmanTree huffmantree(m);
    Node *node = huffmantree.root;

    //读取01字符串
    while (true)
    {
        if (fread(&ch, sizeof(char), 1, src) == 0)
            break;
        // src.read(&ch, 1);
        bits = ch;
        filestream += bits.to_string();
        // if(src.peek() == EOF)break;
    }

    // 处理末尾八位不够的情况
    string end = filestream.substr(filestream.size() - 16, 16);
    bitset<8> endloc(end, 8, 16);
    ulong add = endloc.to_ulong();
    if(add != 0){
        end = end.substr(8 - add, add);
        filestream.erase(filestream.size() - 16, 16);
        filestream += end;
    }else{
        filestream.erase(filestream.size() - 8, 8);
    }

    // 01串遍历哈夫曼树
    for (auto ch : filestream)
    {
        if (ch == '0')
        {
            node = node->left;
        }
        if (ch == '1')
        {
            node = node->right;
        }
        if (node->left == nullptr && node->right == nullptr)
        {
            fwrite(&(node->key), sizeof(char), 1, des);
            // des << node->key;
            //cout << node->key << endl;
            node = huffmantree.root;
        }
    }

    fclose(src);
    fclose(des);

    return 0;
}

int ExportEncrypt::exportContent(int r, int w, const char *keyin)
{
    FILE *src, *des;
    src = fdopen(r, "r");
    des = fdopen(w, "w");

    char ch = 0;
    ch = getc(src);
    if(ch == EOF){
        // cerr << "Error: the file can not be empty!" << endl;
        // exit(1);
        fclose(src);
        fclose(des);
        return 0;
    }
    fseek(src, 0, SEEK_SET);

    // 获取密码字符串，限定长度为32字节
    char key[KEYLEN];
    memset(key, 0, KEYLEN);
    strncpy(key, keyin, KEYLEN);
    unsigned char *aes_key = (unsigned char *)key;

    // 设置输入缓存，大小为1024字节
    char input[STRLEN];
    memset(input, 0, STRLEN);
    unsigned char *aes_input = (unsigned char *)input;

    // 设置初始化向量
    unsigned char iv_enc[AES_BLOCK_SIZE];
    strncpy((char *)iv_enc, key, AES_BLOCK_SIZE);

    // 设置加密输出缓存，大小为1024字节
    unsigned char enc_out[STRLEN];
    memset(enc_out, 0, sizeof(enc_out));
    char *enc = (char *)enc_out;

    // 根据密码字符串生成密钥
    AES_KEY enc_key;
    AES_set_encrypt_key(aes_key, KEYLEN * 8, &enc_key);

    int buflen = 0;
    char bufrem = 0;
    while (true)
    {
        memset(input, 0, STRLEN);
        memset(enc, 0, STRLEN);

        if ((buflen = fread(input, sizeof(char), STRLEN, src)) == 0)
            break;
        // buflen = src.read(input, STRLEN).gcount();
        AES_cbc_encrypt(aes_input, enc_out, buflen, &enc_key, iv_enc, AES_ENCRYPT);
        fwrite(enc, sizeof(char), (buflen == STRLEN) ? STRLEN : ((buflen + 16) - buflen % 16), des);
        // des.write(enc, (buflen == STRLEN) ? STRLEN : ((buflen + 16) - buflen % 16));
        if (buflen != STRLEN)
        {
            bufrem = char(16 - buflen % 16);
            fwrite(&bufrem, sizeof(char), 1, des);
            // des.write(&bufrem, 1);
        }

        // if(src.peek() == EOF) break;
    }

    fclose(src);
    fclose(des);

    return 0;
}

int ExportDecrypt::exportContent(int r, int w, const char *keyin)
{
    FILE *src, *des;
    src = fdopen(r, "r");
    des = fdopen(w, "w");

    char ch = 0;
    ch = getc(src);
    if(ch == EOF){
        // cerr << "Error: the file can not be empty!" << endl;
        // exit(1);
        fclose(src);
        fclose(des);
        return 0;
    }
    fseek(src, 0, SEEK_SET);

    // 获取密码字符串，限定长度为32字节
    char key[KEYLEN];
    memset(key, 0, KEYLEN);
    strncpy(key, keyin, KEYLEN);
    unsigned char *aes_key = (unsigned char *)key;

    // 设置输入缓存，大小为1024字节
    char input[STRLEN];
    memset(input, 0, STRLEN);
    unsigned char *aes_input = (unsigned char *)input;

    // 设置初始化向量
    unsigned char iv_dec[AES_BLOCK_SIZE];
    strncpy((char *)iv_dec, key, AES_BLOCK_SIZE);

    // 设置加密输出缓存，大小为1024字节
    unsigned char dec_out[STRLEN];
    memset(dec_out, 0, sizeof(dec_out));
    char *dec = (char *)dec_out;

    AES_KEY dec_key;
    AES_set_decrypt_key(aes_key, KEYLEN * 8, &dec_key);

    int buflen = 0;
    char bufrem = 0;
    while (true)
    {
        memset(input, 0, STRLEN);
        memset(dec, 0, STRLEN);
        if ((buflen = fread(input, sizeof(char), STRLEN, src)) == 0)
            break;
        // buflen = src.read(input, STRLEN).gcount();
        if (buflen != STRLEN)
        {
            bufrem = input[--buflen];
        }
        AES_cbc_encrypt(aes_input, dec_out, buflen, &dec_key, iv_dec, AES_DECRYPT);
        fwrite(dec, sizeof(char), buflen - (int)bufrem, des);
        // des.write(dec, buflen - (int)bufrem);

        // if(src.peek() == EOF) break;
    }
    fclose(src);
    fclose(des);

    return 0;
}

int ExportEncodeEncrypt::exportContent(int src, int tgt, const char* passwd)
{
    // skip pipeline files
    struct stat file_stat;
    fstat(src, &file_stat);
    if (S_ISFIFO(file_stat.st_mode))
    {
        return 0;
    }

    // tmp file
    char tmp_file[] = "FileBackup_XXXXXX";
    int tmp_fd = mkstemp(tmp_file);

    ExportCompress encoder;
    ExportEncrypt encrypt;

    int encode_fd = open(tmp_file, O_WRONLY);
    encoder.exportContent(src, encode_fd, passwd);
    close(encode_fd);

    int encrypt_fd = open(tmp_file, O_RDONLY);
    encrypt.exportContent(encrypt_fd, tgt, passwd);
    close(encrypt_fd);

    close(tmp_fd);
    remove(tmp_file);
    return 0;
}

int ExportDecodeDecrypt::exportContent(int src, int tgt, const char* passwd)
{
    // skip pipeline files
    struct stat file_stat;
    fstat(src, &file_stat);
    if (S_ISFIFO(file_stat.st_mode))
    {
        return 0;
    }

    // tmp file
    char tmp_file[] = "FileBackup_XXXXXX";
    int tmp_fd = mkstemp(tmp_file);

    ExportDecompress decoder;
    ExportDecrypt decrypt;

    int decrypt_fd = open(tmp_file, O_WRONLY);
    decrypt.exportContent(src, decrypt_fd, passwd);
    close(decrypt_fd);

    int decode_fd = open(tmp_file, O_RDONLY);
    decoder.exportContent(decode_fd, tgt, passwd);
    close(decode_fd);

    close(tmp_fd);
    remove(tmp_file);
    return 0;
}

/* 测试代码： 加密解密文件 */
// int main(int argc, char const *argv[])
// {
//     char key[KEYLEN];

//     cout << "请输入压缩密码： ";
//     cin.getline(key, KEYLEN + 1);

//     // ifstream readfile;
//     // ofstream writefile;
//     int src, des;

//     // readfile.open("a.txt", ios::binary | ios::in);
//     // writefile.open("b.txt", ios::binary | ios::out);
//     src = open("a.txt", O_RDONLY);
//     des = open("b.txt", O_WRONLY | O_CREAT | O_TRUNC);

//     ExportEncrypt encrypt;
//     encrypt.exportContent(src, des, key);

//     // readfile.close();
//     // writefile.close();
//     close(src);
//     close(des);

//     cout << "请输入解压密码： ";
//     cin.getline(key, KEYLEN + 1);

//     // readfile.open("b.txt", ios::binary | ios::in);
//     // writefile.open("c.txt", ios::binary | ios::out);
//     src = open("b.txt", O_RDONLY);
//     des = open("c.txt", O_WRONLY | O_CREAT | O_TRUNC);

//     ExportDecrypt decrypt;
//     decrypt.exportContent(src, des, key);

//     // readfile.close();
//     // writefile.close();
//     close(src);
//     close(des);

//     return 0;
// }

/* 测试代码：压缩解压文件*/
// int main(int argc, char const *argv[])
// {

//     int src, des;
//     char key[] = "123123";

//     src = open("a.txt", O_RDONLY);
//     des = open("b.txt", O_WRONLY | O_CREAT |O_TRUNC);

//     ExportCompress compress;
//     compress.exportContent(src, des, key);

//     close(src);
//     close(des);

//     src = open("b.txt", O_RDONLY);
//     des = open("c.txt", O_WRONLY | O_CREAT | O_TRUNC);

//     ExportDecompress decompress;
//     decompress.exportContent(src, des, key);

//     close(src);
//     close(des);

//     return 0;
// }

// 命令行编译命令：g++ test.cpp -o test -lssl -lcrypto