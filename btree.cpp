//
// Created by levpo on 22.11.2020.
//

#include <iostream>
#include <vector>
#include <malloc.h>
#include <chrono>
#include <memory.h>

#define NIL -1e9-1

using namespace std;

const int t = 50;
const int NODE_SIZE = sizeof(int) + (t * 2) * sizeof(int) + sizeof(bool) + (t * 2 - 1) * sizeof(int) * 2 + sizeof(int);

struct Node {
    int ind;
    int *childs;
    bool leaf;
    int *keys;
    int *values;
    int n;
};

struct BTree {
private:
    FILE *fp = nullptr;
    int _counter = 0;
    int _size = 0;
    Node *root = nullptr;
    long long _timeForAlloc = 0;
    long long _timeForDealloc = 0;
    long long _timeForIO = 0;

    Node *allocateNode() {
        chrono::steady_clock::time_point begin = chrono::steady_clock::now();

        Node *x = new Node;
        x->childs = new int[t * 2];
        x->keys = new int[t * 2 - 1];
        x->values = new int[t * 2 - 1];

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        _timeForAlloc += chrono::duration_cast<chrono::microseconds>(end - begin).count();
        return x;
    }

    void deallocateNode(Node *x) {
        if (!x)
            return;
        chrono::steady_clock::time_point begin = chrono::steady_clock::now();

        delete[] x->childs;
        delete[] x->keys;
        delete[] x->values;
        delete x;

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        _timeForDealloc += chrono::duration_cast<chrono::microseconds>(end - begin).count();
    }

    Node *readNode(int ind) {
        chrono::steady_clock::time_point begin = chrono::steady_clock::now();

        int filePos = ind * NODE_SIZE;
        fseek(fp, filePos, SEEK_SET);
        Node *x = allocateNode();
        fread(&(x->ind), sizeof(int), 1, fp);
        fread(x->childs, sizeof(int), t * 2, fp);
        fread(&(x->leaf), sizeof(bool), 1, fp);
        fread(x->keys, sizeof(int), t * 2 - 1, fp);
        fread(x->values, sizeof(int), t * 2 - 1, fp);
        fread(&(x->n), sizeof(int), 1, fp);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        _timeForIO += chrono::duration_cast<chrono::microseconds>(end - begin).count();
        return x;
    }

    void writeNode(Node *x) {
        chrono::steady_clock::time_point begin = chrono::steady_clock::now();

        int filePos = x->ind * NODE_SIZE;
        fseek(fp, filePos, SEEK_SET);
        fwrite(&(x->ind), sizeof(int), 1, fp);
        fwrite(x->childs, sizeof(int), t * 2, fp);
        fwrite(&(x->leaf), sizeof(bool), 1, fp);
        fwrite(x->keys, sizeof(int), t * 2 - 1, fp);
        fwrite(x->values, sizeof(int), t * 2 - 1, fp);
        fwrite(&(x->n), sizeof(int), 1, fp);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        _timeForIO += chrono::duration_cast<chrono::microseconds>(end - begin).count();
    }

    void splitChild(Node *x, Node *y, Node *z, int yi) {
        z->leaf = y->leaf;
        z->n = y->n = t - 1;
        for (int i = 0; i < t - 1; i++) {
            z->keys[i] = y->keys[t + i];
            z->values[i] = y->values[t + i];
        }
        for (int i = 0; i < t; i++) {
            z->childs[i] = y->childs[t + i];
        }
        for (int i = x->n - 1; i >= yi; i--) {
            x->keys[i + 1] = x->keys[i];
            x->values[i + 1] = x->values[i];
        }
        x->keys[yi] = y->keys[t - 1];
        x->values[yi] = y->values[t - 1];
        for (int i = x->n; i >= yi + 1; i--) {
            x->childs[i + 1] = x->childs[i];
        }
        x->childs[yi + 1] = z->ind;
        x->n++;
        writeNode(x);
        writeNode(y);
        writeNode(z);
    }

    bool insertCorrectly(Node *x, int key, int value) {
        int i = 0;
        while (i < x->n && x->keys[i] < key) {
            i++;
        }
        if (i < x->n && x->keys[i] == key) {
            if (x != root)
                deallocateNode(x);
            return false;
        }
        if (x->leaf) {
            for (int j = x->n - 1; j >= i; j--) {
                x->keys[j + 1] = x->keys[j];
                x->values[j + 1] = x->values[j];
            }
            x->keys[i] = key;
            x->values[i] = value;
            x->n++;
            writeNode(x);
            if (x != root)
                deallocateNode(x);
            return true;
        }
        Node *y = readNode(x->childs[i]);
        if (y->n == t * 2 - 1) {
            Node *z = allocateNode();
            z->ind = _counter++;
            splitChild(x, y, z, i);
            if (key == x->keys[i]) {
                if (x != root)
                    deallocateNode(x);
                deallocateNode(y);
                deallocateNode(z);
                return false;
            }
            if (key > x->keys[i]) {
                swap(y, z);
            }
            deallocateNode(z);
        }
        if (x != root)
            deallocateNode(x);
        return insertCorrectly(y, key, value);
    }

    int find(Node *x, int key) {
        int i = 0;
        while (i < x->n && x->keys[i] < key) {
            i++;
        }
        if (i < x->n && x->keys[i] == key) {
            if (x != root)
                deallocateNode(x);
            return x->values[i];
        }
        if (x->leaf) {
            if (x != root)
                deallocateNode(x);
            return NIL;
        }
        Node *y = readNode(x->childs[i]);
        if (x != root)
            deallocateNode(x);
        return find(y, key);
    }

    void takeFromLeftChild(Node *x, Node *y, Node *z, int yi) {
        for (int i = y->n - 1; i >= 0; i--) {
            y->keys[i + 1] = y->keys[i];
            y->values[i + 1] = y->values[i];
        }
        y->keys[0] = x->keys[yi - 1];
        y->values[0] = x->values[yi - 1];
        for (int i = y->n; i >= 0; i--) {
            y->childs[i + 1] = y->childs[i];
        }
        y->childs[0] = z->childs[z->n];
        y->n++;
        x->keys[yi - 1] = z->keys[z->n - 1];
        x->values[yi - 1] = z->values[z->n - 1];
        z->n--;
        writeNode(x);
        writeNode(y);
        writeNode(z);
    }

    void takeFromRightChild(Node *x, Node *y, Node *z, int yi) {
        y->keys[y->n] = x->keys[yi];
        y->values[y->n] = x->values[yi];
        y->childs[y->n + 1] = z->childs[0];
        y->n++;
        x->keys[yi] = z->keys[0];
        x->values[yi] = z->values[0];
        for (int i = 0; i < z->n - 1; i++) {
            z->keys[i] = z->keys[i + 1];
            z->values[i] = z->values[i + 1];
        }
        for (int i = 0; i < z->n; i++) {
            z->childs[i] = z->childs[i + 1];
        }
        z->n--;
        writeNode(x);
        writeNode(y);
        writeNode(z);
    }

    void mergeChilds(Node *x, Node *y, Node *z, int yi) {
        y->keys[y->n] = x->keys[yi];
        y->values[y->n] = x->values[yi];
        for (int i = 0; i < z->n; i++) {
            y->keys[y->n + 1 + i] = z->keys[i];
            y->values[y->n + 1 + i] = z->values[i];
        }
        for (int i = 0; i <= z->n; i++) {
            y->childs[y->n + 1 + i] = z->childs[i];
        }
        y->n += 1 + z->n;
        for (int i = yi; i < x->n - 1; i++) {
            x->keys[i] = x->keys[i + 1];
            x->values[i] = x->values[i + 1];
        }
        for (int i = yi + 1; i < x->n; i++) {
            x->childs[i] = x->childs[i + 1];
        }
        x->n--;
        z->n = 0;
        writeNode(x);
        writeNode(y);
        writeNode(z);
    }

    pair<int, int> deleteMaxKey(Node *x) {
        if (x->leaf) {
            pair<int, int> pa = {x->keys[x->n - 1], x->values[x->n - 1]};
            x->n--;
            writeNode(x);
            if (x != root)
                deallocateNode(x);
            return pa;
        }
        Node *y = readNode(x->childs[x->n]);
        if (y->n == t - 1) {
            Node *z = readNode(x->childs[x->n - 1]);
            if (z->n > t - 1) {
                takeFromLeftChild(x, y, z, x->n);
            } else {
                swap(y, z);
                mergeChilds(x, y, z, x->n - 1);
            }
            deallocateNode(z);
        }
        if (x != root)
            deallocateNode(x);
        return deleteMaxKey(y);
    }

    pair<int, int> deleteMinKey(Node *x) {
        if (x->leaf) {
            pair<int, int> pa = {x->keys[0], x->values[0]};
            for (int i = 0; i < x->n - 1; i++) {
                x->keys[i] = x->keys[i + 1];
                x->values[i] = x->values[i + 1];
            }
            x->n--;
            writeNode(x);
            if (x != root)
                deallocateNode(x);
            return pa;
        }
        Node *y = readNode(x->childs[0]);
        if (y->n == t - 1) {
            Node *z = readNode(x->childs[1]);
            if (z->n > t - 1) {
                takeFromRightChild(x, y, z, 0);
            } else {
                mergeChilds(x, y, z, 0);
            }
            deallocateNode(z);
        }
        if (x != root)
            deallocateNode(x);
        return deleteMinKey(y);
    }

    int deleteCorrectly(Node *x, int key) {
        int i = 0;
        while (i < x->n && x->keys[i] < key) {
            i++;
        }
        if (i < x->n && x->keys[i] == key) {
            int res = x->values[i];
            if (x->leaf) {
                for (int j = i; j < x->n - 1; j++) {
                    x->keys[j] = x->keys[j + 1];
                    x->values[j] = x->values[j + 1];
                }
                x->n--;
                writeNode(x);
                if (x != root)
                    deallocateNode(x);
                return res;
            }
            Node *y = readNode(x->childs[i]);
            if (y->n > t - 1) {
                pair<int, int> pa = deleteMaxKey(y);
                x->keys[i] = pa.first;
                x->values[i] = pa.second;
                writeNode(x);
                if (x != root)
                    deallocateNode(x);
                return res;
            }
            Node *z = readNode(x->childs[i + 1]);
            if (z->n > t - 1) {
                pair<int, int> pa = deleteMinKey(z);
                x->keys[i] = pa.first;
                x->values[i] = pa.second;
                writeNode(x);
                if (x != root)
                    deallocateNode(x);
                deallocateNode(y);
                return res;
            }
            mergeChilds(x, y, z, i);
            if (x != root)
                deallocateNode(x);
            deallocateNode(z);
            return deleteCorrectly(y, key);
        }
        if (x->leaf) {
            if (x != root)
                deallocateNode(x);
            return NIL;
        }
        Node *y = readNode(x->childs[i]);
        if (y->n == t - 1) {
            bool ok = false;
            Node *z1 = nullptr, *z2 = nullptr;
            if (i > 0) {
                z1 = readNode(x->childs[i - 1]);
                if (z1->n > t - 1) {
                    takeFromLeftChild(x, y, z1, i);
                    deallocateNode(z1);
                    ok = true;
                }
            }
            if (!ok && i < x->n) {
                z2 = readNode(x->childs[i + 1]);
                if (z2->n > t - 1) {
                    takeFromRightChild(x, y, z2, i);
                    deallocateNode(z2);
                    ok = true;
                }
            }
            if (!ok) {
                if (i > 0) {
                    swap(y, z1);
                    mergeChilds(x, y, z1, i - 1);
                } else {
                    mergeChilds(x, y, z2, i);
                }
                deallocateNode(z1);
                deallocateNode(z2);
            }
        }
        if (x != root)
            deallocateNode(x);
        return deleteCorrectly(y, key);
    }


public:
    BTree() {
        fp = fopen("C:\\Temp\\data.bin", "w+b");
        root = allocateNode();
        root->ind = _counter++;
        root->leaf = true;
        root->n = 0;
        writeNode(root);
    }

    ~BTree() {
        deallocateNode(root);
        fclose(fp);
    }

    bool doInsert(int key, int value) {
        if (root->n == t * 2 - 1) {
            Node *x = root;
            root = allocateNode();
            root->ind = _counter++;
            root->leaf = false;
            root->n = 0;
            root->childs[0] = x->ind;
            Node *y = allocateNode();
            y->ind = _counter++;
            splitChild(root, x, y, 0);
            deallocateNode(x);
            deallocateNode(y);
        }
        bool res = insertCorrectly(root, key, value);
        if (res)
            _size++;
        return res;
    }

    int doFind(int key) {
        return find(root, key);
    }

    int doDelete(int key) {
        int res = deleteCorrectly(root, key);
        if (root->n == 0 && !root->leaf) {
            Node *x = root;
            root = readNode(root->childs[0]);
            deallocateNode(x);
        }
        if (res != NIL)
            _size--;
        return res;
    }

    int size() {
        return _size;
    }

    int counter() {
        return _counter;
    }

    long long timeForAlloc() {
        return _timeForAlloc;
    }

    long long timeForDealloc() {
        return _timeForDealloc;
    }

    long long timeForIO() {
        return _timeForIO;
    }
};
