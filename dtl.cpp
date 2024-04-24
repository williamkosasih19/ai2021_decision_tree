#include "dtl.h"

static double CalculateGain(const string category,
                     const double splitVal,
                     vector<map<string, double>> &data)
{
  const auto calculateEntropy = [&](vector<map<string, double>> &data) {
    map<double, uint32_t> categoriesMap;
    for (map<string, double> &datum : data)
    {
      if (categoriesMap.find(datum["quality"]) ==
          categoriesMap.end())
      {
        categoriesMap[datum["quality"]] = 0;
      }
      categoriesMap[datum["quality"]]++;
    }
    double entropy = 0.0;
    for (const pair<double, uint32_t> &p : categoriesMap)
    {
      const double categoryProb = double(p.second) / double(data.size());
      entropy += (-categoryProb * log2(categoryProb));
    }
    return entropy;
  };
  const double rootEntropy = calculateEntropy(data);

  vector<map<string, double>> leftData;
  vector<map<string, double>> rightData;

  for (map<string, double> datum : data)
  {
    if (datum[category] <= splitVal)
      leftData.push_back(datum);
    else
      rightData.push_back(datum);
  }

  const double leftProbability = double(leftData.size()) / double(data.size());
  const double rightProbability = double(rightData.size()) / double(data.size());

  const double leftEntropy = calculateEntropy(leftData);
  const double rightEntropy = calculateEntropy(rightData);

  const double remainder =
      leftEntropy * leftProbability + rightEntropy * rightProbability;

  return rootEntropy - remainder;
}

static pair<string, double> ChooseSplit(vector<map<string, double>> &data)
{
  double bestGain = 0.0;
  string bestAttr;
  double bestSplitVal;

  for (const pair<string, double> &p : data[0])
  {
    const string attr = p.first;
    if (attr == "quality")
      continue;

    vector<double> attrVec;
    for (map<string, double> &datum : data)
    {
      attrVec.push_back(datum[attr]);
    }
    sort(attrVec.begin(), attrVec.end());

    for (uint32_t i = 0; i < attrVec.size() - 1; i++)
    {
      const double splitVal = (attrVec[i] + attrVec[i + 1]) / 2;
      const double gain = CalculateGain(attr, splitVal, data);
      if (gain >= bestGain)
      {
        bestGain = gain;
        bestAttr = attr;
        bestSplitVal = splitVal;
      }
    }
  }
  return pair<string, double>(bestAttr, bestSplitVal);
}

static double CountMajority(vector<map<string, double>> &data)
{
  map<double, uint32_t> qualityCount;
  for (map<string, double> &datum : data)
  {
    if (qualityCount.find(datum["quality"]) == qualityCount.end())
      qualityCount[datum["quality"]] = 0;
    qualityCount[datum["quality"]]++;
  }

  vector<pair<double, uint32_t>> qualityCountVec(qualityCount.begin(),
                                                 qualityCount.end());
  sort(qualityCountVec.begin(), qualityCountVec.end(),
       [&](const pair<double, uint32_t> &p1, const pair<double, uint32_t> &p2) {
         return p1.second > p2.second;
       });

  if (qualityCountVec.size() == 1 || qualityCountVec[0].second > qualityCountVec[1].second)
    return qualityCountVec[0].first;
  else
    return LABEL_UNKNOWN;
}

Node *DTL(vector<map<string, double>> data, const uint32_t minLeaf)
{
  const uint32_t N = data.size();

  vector<string> m1Keys;
  transform(data.front().begin(), data.front().end(), back_inserter(m1Keys),
            [&](const pair<string, double> &p) {
              return p.first;
            });
  const double firstQuality = data.front()["quality"];

  if (N <= minLeaf ||
      all_of(data.begin(), data.end(),
             [&](map<string, double> &m2) {
               return m2["quality"] == firstQuality;
             }) ||
      data.front().size() == 1)
  {
    Node *tempNode = new Node();

    tempNode->label = CountMajority(data);

    return tempNode;
  }
  const pair<string, double> split = ChooseSplit(data);

  Node *tempNode = new Node();
  tempNode->attr = split.first;
  tempNode->splitVal = split.second;

  vector<map<string, double>> leftData;
  vector<map<string, double>> rightData;

  for (map<string, double> datum : data)
  {
    if (datum[tempNode->attr] <= tempNode->splitVal)
      leftData.push_back(datum);
    else
      rightData.push_back(datum);
  }

  tempNode->label = CountMajority(data);

  tempNode->left = unique_ptr<Node>(DTL(leftData, minLeaf));
  tempNode->right = unique_ptr<Node>(DTL(rightData, minLeaf));

  return tempNode;
}

double PredictDTL(const Node *node, map<string, double> &datum)
{
  while (true)
  {
    if (datum[node->attr] <= node->splitVal)
    {
      if (node->left.get() == nullptr)
        break;
      node = node->left.get();
    }
    else
    {
      if (node->right.get() == nullptr)
        break;
      node = node->right.get();
    }
  }
  return node->label;
}

vector<string> ParseLine(string lineString)
{
  vector<string> resultVector;
  istringstream isstream(lineString);
  while (isstream)
  {
    string temp;
    isstream >> skipws >> temp;

    if (temp == "")
      continue;

    resultVector.push_back(temp);
  }
  return resultVector;
}

vector<map<string, double>> createData (string dFileName) {
  ifstream ifs(dFileName);

  string line;
  getline(ifs, line);
  vector<string> attributes = ParseLine(line);

  vector<map<string, double>> data;

  while (getline(ifs, line))
  {
    vector<string> lineStringVector = ParseLine(line);
    map<string, double> datum;

    for (uint32_t i = 0; i < attributes.size(); i++)
      datum[attributes[i]] = stod(lineStringVector[i]);

    data.push_back(datum);
  }
  ifs.close();
  return data;
};