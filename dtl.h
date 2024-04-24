#pragma once

#include <map>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstdint>
#include <memory>
#include <queue>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>


#define LABEL_UNKNOWN -3.0
#define LABEL_LEAF -4.0


using namespace std;

struct Node
{
  string attr;
  double label = LABEL_LEAF;
  double splitVal;

  unique_ptr<Node> left = nullptr;
  unique_ptr<Node> right = nullptr;
};

Node *DTL(vector<map<string, double>> data, const uint32_t minLeaf);
vector<string> ParseLine(string lineString);
double PredictDTL(const Node *node, map<string, double> &datum);
vector<map<string, double>> createData (string dFileName);