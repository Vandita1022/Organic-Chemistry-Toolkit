#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <stack>
#include <sstream>

using namespace std;

class CarbonNode {
public:
    int id;
    int C_C_bonds = 0;
    int C_H_bonds = 0;
    int C_X_bonds = 0;
    string label;

    CarbonNode() : id(0), label("C") {}

    CarbonNode(int id, const string& label = "C") : id(id), label(label) {}

    void incrementC_C() { C_C_bonds++; }
    void incrementC_H(int numH) { C_H_bonds = numH; }
    void incrementC_X() { C_X_bonds++; }

    int getTotalBonds() const { return C_C_bonds + C_H_bonds + C_X_bonds; }

    void printInfo() const {
        cout << label << id << ": C-C=" << C_C_bonds << ", C-H=" << C_H_bonds << ", C-X=" << C_X_bonds << "; ";
    }
};

class MolecularGraph {
public:
    unordered_map<int, CarbonNode> carbons;
    unordered_map<int, vector<int>> adjacencyList;
    vector<pair<int, int>> edges;
    vector<pair<string, string>> input;
    
    int counter = 1;

    void addCarbon(const string& label) {
        carbons[counter] = CarbonNode(counter, label);
        counter++;
    }

    void addEdge(int id1, int id2) {
        carbons[id1].incrementC_C();
        carbons[id2].incrementC_C();
        edges.emplace_back(id1, id2);
        adjacencyList[id1].push_back(id2);
        adjacencyList[id2].push_back(id1);
    }

    void parseMolecularFormula(const string& formula) {
        stack<int> branchPoints;
        int previousCarbon = 0;

        for (size_t i = 0; i < formula.size();) {
            char ch = formula[i];
            string label(1, ch);

            if (ch == 'C') {
                if (i + 1 < formula.size() && formula[i + 1] == 'l') {
                    label = "Cl";
                    carbons[counter - 1].incrementC_X();
                    i++;
                } else if (i + 1 < formula.size() && formula[i + 1] == 'O') {
                    label = "COOH";
                    i += 3;
                }

                addCarbon(label);
                int currentCarbon = counter - 1;

                if (i + 1 < formula.size() && formula[i + 1] == 'H') {
                    int numH = 1;
                    i++;
                    if (i + 1 < formula.size() && isdigit(formula[i + 1])) {
                        numH = formula[i + 1] - '0';
                        i++;
                    }
                    carbons[currentCarbon].incrementC_H(numH);
                }

                if (previousCarbon != 0) {
                    addEdge(previousCarbon, currentCarbon);
                }

                previousCarbon = currentCarbon;
                i++;
            } else if (ch == '(') {
                branchPoints.push(previousCarbon);
                i++;
            } else if (ch == ')') {
                if (!branchPoints.empty()) {
                    previousCarbon = branchPoints.top();
                    branchPoints.pop();
                }
                i++;
            } else {
                if (isalpha(ch)) {
                    string halogenLabel(1, ch);
                    if (i + 1 < formula.size() && islower(formula[i + 1])) {
                        halogenLabel += formula[i + 1];
                        i++;
                    }
                    addCarbon(halogenLabel);
                    int currentAtom = counter - 1;

                    previousCarbon = currentAtom;
                    i++;
                }
            }
        }
    }

    bool hasCyclicEdge() {
        vector<int> candidates;
        for (const auto& pair : carbons) {
            if (pair.second.getTotalBonds() == 3) {
                candidates.push_back(pair.first);
            }
        }

        if (candidates.size() >= 2) {
            addEdge(candidates[0], candidates[1]);
            cout << "Added cyclic edge between nodes " << candidates[0] << " and " << candidates[1] << endl;
            cout << endl;
            return true;
        }
        return false;
    }

    void printAtomsInfo() const {
        cout << "Atoms Info" << endl;
        for (const auto& pair : carbons) {
            pair.second.printInfo();
            cout << endl;
        }
        cout << endl;
    }

    void printEdges()
    {
        cout << "Edges" << endl;
        for (const auto& edge : edges) {
            cout << carbons.at(edge.first).label << edge.first << "-"
                 << carbons.at(edge.second).label << edge.second << endl;
            
            string label1 = carbons.at(edge.first).label + to_string(edge.first);
            string label2 = carbons.at(edge.second).label + to_string(edge.second);
            input.emplace_back(label1, label2);
        }
        cout << endl;
    }
};

// -------------------- Helper Functions --------------------

// Graph represented as an adjacency list
unordered_map<int, vector<int>> graph;

// Function to add an edge to the graph
void addEdge(int u, int v) {
    graph[u].push_back(v);
    graph[v].push_back(u);
}

// Modified DFS to track path
pair<int, vector<int>> dfsWithConditions(int node, unordered_set<int>& visited, const unordered_set<int>& ignoredNodes) {
    visited.insert(node);
    int maxLength = 0;
    vector<int> longestPath = {node};

    for (int neighbor : graph[node]) {
        if (visited.find(neighbor) == visited.end() && ignoredNodes.find(neighbor) == ignoredNodes.end()) {
            auto [length, path] = dfsWithConditions(neighbor, visited, ignoredNodes);
            if (length + 1 > maxLength) {
                maxLength = length + 1;
                longestPath = {node};
                longestPath.insert(longestPath.end(), path.begin(), path.end());
            }
        }
    }
    visited.erase(node); // Backtrack
    return {maxLength, longestPath};
}

// Function to find the longest carbon chain with path tracking
vector<int> findLongestCarbonChain(int startNode, const unordered_set<int>& ignoredNodes) {
    unordered_set<int> visited;

    // Step 1: First DFS to find the farthest node from startNode
    auto [_, farthestNodePath] = dfsWithConditions(startNode, visited, ignoredNodes);
    int farthestNode = farthestNodePath.back();

    // Step 2: Second DFS from the farthest node found in the first DFS
    visited.clear();
    auto [longestChainLength, longestChainPath] = dfsWithConditions(farthestNode, visited, ignoredNodes);

    return longestChainPath; // Returns the path (chain of nodes) in one direction
}

vector<int> getOptimalChainDirection(const vector<int>& chain, const unordered_map<int, vector<int>>& branchInfo) {
    vector<int> leftLocants, rightLocants;

    // Calculate left-to-right locants for branches
    for (size_t i = 0; i < chain.size(); i++) {
        int atom = chain[i];
        if (branchInfo.find(atom) != branchInfo.end()) {
            leftLocants.push_back(i + 1); // Locants from left side
        }
    }

    // Calculate right-to-left locants for branches
    for (size_t i = 0; i < chain.size(); i++) {
        int atom = chain[chain.size() - i - 1];
        if (branchInfo.find(atom) != branchInfo.end()) {
            rightLocants.push_back(i + 1); // Locants from right side
        }
    }

    // Perform lexicographical comparison
    if (leftLocants < rightLocants) {
        return chain; // Left-to-right is lexicographically smaller
    } else {
        return vector<int>(chain.rbegin(), chain.rend()); // Reverse for right-to-left
    }
}



string formatBranchName(int numCarbons, int halogenType) {
    string branchName;

    // Set the branch name based on the number of carbons
    if (numCarbons == 1) {
        branchName = "methyl";
    } else if (numCarbons == 2) {
        branchName = "ethyl";
    } else if (numCarbons == 3) {
        branchName = "propyl";
    } else if (numCarbons == 4) {
        branchName = "butyl";
    } 
    // else {
    //     branchName = to_string(numCarbons) + "-butyl";  // For simplicity, use "butyl" as a generic name for larger chains
    // }

    // Add the specific halogen prefix if halogenType is set
    if (halogenType > 0) {
        switch (halogenType) {
            case 1:
                branchName = "chloro-" + branchName;
                break;
            case 2:
                branchName = "bromo-" + branchName;
                break;
            // Add other cases for additional halogens as needed
            default:
                branchName = "halo-" + branchName;  // Fallback for unspecified halogens
        }
    }

    return branchName;
}


/// Helper function to combine branches with identical names and add prefixes for duplicates
vector<string> combineBranches(const vector<string>& branches) {
    vector<string> combinedBranches;
    int i = 0;

    while (i < branches.size()) {
        // Extract locant and branch name
        size_t dashPos = branches[i].find('-');
        string locant = branches[i].substr(0, dashPos);
        string branchName = branches[i].substr(dashPos + 1);

        vector<string> locants = {locant};

        // Check for identical consecutive branch names and collect their locants
        while (i + 1 < branches.size() && branches[i + 1].find("-" + branchName) != string::npos) {
            size_t nextDashPos = branches[i + 1].find('-');
            string nextLocant = branches[i + 1].substr(0, nextDashPos);
            locants.push_back(nextLocant);
            i++;
        }

        // Combine locants if there are multiple, otherwise keep single locant
        if (locants.size() > 1) {
            string prefix = ""; // Prefix for naming based on the count
            if (locants.size() == 2) {
                prefix = "di";
            } else if (locants.size() == 3) {
                prefix = "tri";
            } else if (locants.size() > 3) {
                prefix = to_string(locants.size()) + "-";
            }

            string combinedLocants = "(" + locants[0];
            for (size_t j = 1; j < locants.size(); j++) {
                combinedLocants += "," + locants[j];
            }
            combinedLocants += ")";
            combinedBranches.push_back(combinedLocants + "-" + prefix + branchName);
        } else {
            combinedBranches.push_back(locants[0] + "-" + branchName);
        }

        i++;
    }

    return combinedBranches;
}



string generateIUPACName(const vector<int>& longestChain, const unordered_map<int, string>& idToLabel, unordered_map<int, vector<int>>& branchInfo, int counter) {
    int numCarbons = longestChain.size();
    string chainName;
    if (numCarbons == 1) chainName = "Meth";
    else if (numCarbons == 2) chainName = "Eth";
    else if (numCarbons == 3) chainName = "Prop";
    else if (numCarbons == 4) chainName = "But";
    else if (numCarbons == 5) chainName = "Pent";
    else if (numCarbons == 6) chainName = "Hex";
    else if (numCarbons == 7) chainName = "Hept";
    else if (numCarbons == 8) chainName = "Oct";
    else if (numCarbons == 9) chainName = "Non";
    else if (numCarbons == 10) chainName = "Dec";

    if(counter==0)
    {
        chainName = chainName + "ane";
    }
    else if (counter==1)
    {
        chainName = chainName + "an";
    }
    else if(counter==2)
    {
        chainName = chainName + "yl";
    }
    

    // Generate branch names with locants
    vector<string> branches;
    for (size_t i = 0; i < longestChain.size(); i++) {
        int atom = longestChain[i];
        if (branchInfo.find(atom) != branchInfo.end()) {
            int numCarbonsInBranch = branchInfo[atom][0];
            int halogenType = branchInfo[atom][1];
            string branchName = formatBranchName(numCarbonsInBranch, halogenType);
            int locant = i + 1;  // 1-based locant
            branches.push_back(to_string(locant) + "-" + branchName);
        }
    }

    // Sort branches by locants to ensure correct order
    sort(branches.begin(), branches.end());

    // Combine branches with identical names
    vector<string> combinedBranches = combineBranches(branches);

    // Combine branch information with the main chain name
    string finalName;
    for (const string& branch : combinedBranches) {
        finalName += branch + " ";
    }
    finalName += chainName; // Append main chain name after branches

    return finalName;
}


#include <stack>

// Helper function to count carbons and detect halogens in a branch starting from a given node
pair<int, int> countBranchCarbons(int start, const unordered_set<int>& mainChainNodes, const unordered_map<int, string>& idToLabel) {
    unordered_set<int> visited;  // Track visited nodes within the branch
    stack<int> toVisit;
    toVisit.push(start);

    int carbonCount = 0;
    int halogenType = 0;  // 0 indicates no halogen; 1 for chlorine, 2 for bromine, etc.

    // Iterative DFS to explore the branch fully
    while (!toVisit.empty()) {
        int node = toVisit.top();
        toVisit.pop();

        // Get the label for the current node
        string label = idToLabel.at(node);

        // Debugging print to see the label being processed
        cout << "Processing label: " << label << endl;

        // Check if the node is a carbon (must start with "C" followed by a number)
        if (label[0] == 'C' && label.substr(1).find_first_not_of("0123456789") == string::npos) {
            carbonCount++;
        } 
        // Check if the node is a halogen (specific labels that start with Cl, Br, F, I, etc.)
        else if (label.substr(0, 2) == "Cl" && label.length() > 2 && isdigit(label[2]) && halogenType == 0) {
            halogenType = 1;  // Chlorine
        } else if (label.substr(0, 2) == "Br" && label.length() > 2 && isdigit(label[2]) && halogenType == 0) {
            halogenType = 2;  // Bromine
        } else if (label.substr(0, 1) == "F" && label.length() > 1 && isdigit(label[1]) && halogenType == 0) {
            halogenType = 3;  // Fluorine
        } else if (label.substr(0, 1) == "I" && label.length() > 1 && isdigit(label[1]) && halogenType == 0) {
            halogenType = 4;  // Iodine
        }
        // Add more halogens as needed

        // Debug print to check halogenType and carbonCount
        cout << "Detected label: " << label << ", halogenType: " << halogenType << ", carbonCount: " << carbonCount << endl;

        visited.insert(node);  // Mark this node as visited

        // Explore neighbors to find other carbons in the branch
        for (int neighbor : graph[node]) {
            if (visited.find(neighbor) == visited.end() && mainChainNodes.find(neighbor) == mainChainNodes.end()) {
                visited.insert(neighbor);  // Mark the neighbor as visited in the branch
                toVisit.push(neighbor);
            }
        }
    }

    return {carbonCount, halogenType};
}

// Helper function to detect if a node is a COOH group (e.g., COOH1, COOH2)
bool isCOOHGroup(const string& label) {
    return label.substr(0, 4) == "COOH" && label.length() > 4 && isdigit(label[4]);
}

vector<int> findLongestChainWithCOOH(const unordered_set<int>& coohNodes, const unordered_set<int>& ignoredNodes) {
    vector<int> longestChain;

    for (int coohNode : coohNodes) {
        unordered_set<int> visited;
        // Start DFS from the COOH node
        auto [_, path] = dfsWithConditions(coohNode, visited, ignoredNodes);

        // Keep track of the longest path found
        if (path.size() > longestChain.size()) {
            longestChain = path;
        }
    }

    // If the COOH group is at the end of the chain, reverse the chain
    // if (!longestChain.empty() && longestChain.back() == *coohNodes.begin()) {
    reverse(longestChain.begin(), longestChain.end());
    // }

    return longestChain;
}

// Modify the function signature to return a string
string processMolecularGraph(MolecularGraph& graph1, int hint) {
    graph1.printAtomsInfo();
    bool cycle = graph1.hasCyclicEdge();
    if(cycle) return 0;
    graph1.printEdges();

    vector<pair<string, string>> edgeList = graph1.input;

    int counter = 0;

    unordered_map<string, int> labelToID;
    unordered_map<int, string> idToLabel;
    int currentID = 0;
    unordered_set<int> coohNodes;  // Set to store COOH node IDs
    unordered_map<int, vector<int>> branchInfo;

    unordered_set<int> ignoredNodes;

    // Assign IDs and build graph
    for (const auto& edge : edgeList) {
        string nodeA = edge.first;
        string nodeB = edge.second;

        if (labelToID.find(nodeA) == labelToID.end()) {
            labelToID[nodeA] = currentID;
            idToLabel[currentID] = nodeA;
            currentID++;
        }
        if (labelToID.find(nodeB) == labelToID.end()) {
            labelToID[nodeB] = currentID;
            idToLabel[currentID] = nodeB;
            currentID++;
        }

        int idA = labelToID[nodeA];
        int idB = labelToID[nodeB];
        addEdge(idA, idB);

        // Check if the node is a COOH group
        if (isCOOHGroup(nodeA)) {
            coohNodes.insert(idA);
        }
        if (isCOOHGroup(nodeB)) {
            coohNodes.insert(idB);
        }

        // Ignore non-carbon nodes for main chain detection
        if (nodeA[0] != 'C') ignoredNodes.insert(idA);
        if (nodeB[0] != 'C') ignoredNodes.insert(idB);
    }

    vector<int> carbonNodes;
    for (const auto& pair : labelToID) {
        if (pair.first[0] == 'C') {
            carbonNodes.push_back(pair.second);
        }
    }

    if (carbonNodes.empty()) {
        cout << "No carbon atoms found in the input.\n";
        return "";
    }

   // Step 1: If COOH group is found, find the longest chain starting from COOH
    vector<int> longestChain;
    if (!coohNodes.empty()) {
        counter = 1;
        longestChain = findLongestChainWithCOOH(coohNodes, ignoredNodes);
    } else {
        // If no COOH group, find the longest chain normally
        int startNode = carbonNodes[0];
        longestChain = findLongestCarbonChain(startNode, ignoredNodes);
    }

    vector<int> optimalChain = getOptimalChainDirection(longestChain, branchInfo);
    if(hint == 1) counter = 2;

    // Print the longest carbon chain using node labels
    cout << "Longest carbon chain: ";
    for (int node : optimalChain) {
        cout << idToLabel[node] << " ";
    }
    cout << endl;

    // Step 3: Store branch information
    for (int atom : optimalChain) {
        for (int neighbor : graph[atom]) {
            if (ignoredNodes.find(neighbor) == ignoredNodes.end() && find(optimalChain.begin(), optimalChain.end(), neighbor) == optimalChain.end()) {
                // Neighbor is a branch starting point, call countBranchCarbons
                auto [numCarbonsInBranch, halogenPresent] = countBranchCarbons(neighbor, unordered_set<int>(optimalChain.begin(), optimalChain.end()), idToLabel);
                branchInfo[atom] = {numCarbonsInBranch, halogenPresent};
            }
        }
    }

    // Step 4: Generate IUPAC name with '-oic acid' if COOH is present
    string iupacName = generateIUPACName(optimalChain, idToLabel, branchInfo, counter);
    if (!coohNodes.empty()) {
        iupacName += "oic acid";
    }

    // string iupacName = generateIUPACName(optimalChain, idToLabel, branchInfo, counter);
    cout << "IUPAC Name: " << iupacName << endl;

    return iupacName; // Return the IUPAC name for use in ethers
}

// Helper function to generate IUPAC name for a single molecular graph
string generateIUPACNameForGraph(MolecularGraph& graph) {
    return processMolecularGraph(graph, 1); // This will print atom info and IUPAC name internally
    
}


// -------------------- Main Code --------------------

int main() {
    MolecularGraph graph;
    string formula;
    getline(cin, formula);
    cout << endl;

    size_t pos = formula.find('-');
    if(pos != string::npos && pos + 2 < formula.length() && formula[pos + 1] == 'O' && formula[pos + 2] == '-') {
        string f1 = formula.substr(0, pos);
        string f2 = formula.substr(pos + 3);
        
        cout<<f1<<" "<<f2<<endl;

        MolecularGraph g1, g2;
        g1.parseMolecularFormula(f1);
        g2.parseMolecularFormula(f2);
        
        string name1 = generateIUPACNameForGraph(g1);
        string name2 = generateIUPACNameForGraph(g2);

        // Ensure the smaller group name comes first
        if (name1 > name2) {
            swap(name1, name2);
        }

        cout << "IUPAC NAME: " << name1 << " " << name2 << " ether" << endl;
        return 0;
    }

    graph.parseMolecularFormula(formula);
    processMolecularGraph(graph, 0);

    return 0;
}