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

            // --- Handle COOH Group ---
            if (i + 3 < formula.size() && formula.substr(i, 4) == "COOH") {
                addCarbon("COOH");
                int currentCarbon = counter - 1;
                if (previousCarbon != 0) addEdge(previousCarbon, currentCarbon);
                previousCarbon = currentCarbon;
                i += 4;
                continue;
            }

            // --- Handle Carbon Atoms ---
            if (ch == 'C') {
                addCarbon("C");
                int currentCarbon = counter - 1;
                i++;

                // Check for H (e.g., CH3)
                if (i < formula.size() && formula[i] == 'H') {
                    int numH = 1;
                    i++;
                    if (i < formula.size() && isdigit(formula[i])) {
                        numH = formula[i] - '0';
                        i++;
                    }
                    carbons[currentCarbon].incrementC_H(numH);
                }

                // Check for Cl, Br, F, I right after CHx
                if (i + 1 < formula.size()) {
                    string next2 = formula.substr(i, 2);
                    if (next2 == "Cl" || next2 == "Br") {
                        carbons[currentCarbon].incrementC_X();
                        i += 2;
                    } else if (formula[i] == 'F' || formula[i] == 'I') {
                        carbons[currentCarbon].incrementC_X();
                        i++;
                    }
                }

                if (previousCarbon != 0) {
                    addEdge(previousCarbon, currentCarbon);
                }
                previousCarbon = currentCarbon;
                continue;
            }

            // --- Handle Branch Start ---
            if (ch == '(') {
                branchPoints.push(previousCarbon);
                i++;
                continue;
            }

            // --- Handle Branch End ---
            if (ch == ')') {
                if (!branchPoints.empty()) {
                    previousCarbon = branchPoints.top();
                    branchPoints.pop();
                }
                i++;
                continue;
            }

            // --- Handle Halogen as Direct Label (e.g., CH3CH2CH(Cl)CH3) ---
            if (isalpha(ch)) {
                string halogenLabel(1, ch);
                if (i + 1 < formula.size() && islower(formula[i + 1])) {
                    halogenLabel += formula[i + 1];
                }

                if (halogenLabel == "Cl" || halogenLabel == "Br") {
                    if (previousCarbon != 0) {
                        carbons[previousCarbon].incrementC_X();
                    }
                    i += 2;
                } else if (ch == 'F' || ch == 'I') {
                    if (previousCarbon != 0) {
                        carbons[previousCarbon].incrementC_X();
                    }
                    i++;
                } else {
                    // Unknown label – treat as separate carbon or atom (fallback)
                    addCarbon(halogenLabel);
                    int currentAtom = counter - 1;
                    if (previousCarbon != 0) {
                        addEdge(previousCarbon, currentAtom);
                    }
                    previousCarbon = currentAtom;
                    i += halogenLabel.length();
                }
            } else {
                i++; // Skip unknown or malformed characters
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
            pair<int, vector<int>> res = dfsWithConditions(neighbor, visited, ignoredNodes);
            int length = res.first;
            vector<int> path = res.second;

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
    pair<int, vector<int>> res1 = dfsWithConditions(startNode, visited, ignoredNodes);
    vector<int> farthestNodePath = res1.second;

    int farthestNode = farthestNodePath.back();

    // Step 2: Second DFS from the farthest node found in the first DFS
    visited.clear();
    pair<int, vector<int>> res2 = dfsWithConditions(farthestNode, visited, ignoredNodes);
    vector<int> longestChainPath = res2.second;

    return longestChainPath; // Returns the path (chain of nodes) in one direction
}

vector<int> getOptimalChainDirection(const vector<int>& chain, const unordered_map<int, vector<pair<int, int>>>& branchInfo) {
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
    // For halogens, return just the halogen name
    if (halogenType > 0) {
        switch (halogenType) {
            case 1: return "chloro";
            case 2: return "bromo";
            case 3: return "fluoro";
            case 4: return "iodo";
            default: return "halo";
        }
    }

    // For alkyl branches
    string branchName;
    if (numCarbons == 1) {
        branchName = "methyl";
    } else if (numCarbons == 2) {
        branchName = "ethyl";
    } else if (numCarbons == 3) {
        branchName = "propyl";
    } else if (numCarbons == 4) {
        branchName = "butyl";
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



string generateIUPACName(const vector<int>& longestChain, const unordered_map<int, string>& idToLabel, unordered_map<int, vector<pair<int, int>>>& branchInfo, int counter) {
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
            // Process ALL branches for this atom
            for (const pair<int, int>& branch : branchInfo[atom]) {
                int numCarbonsInBranch = branch.first;
                int halogenType = branch.second;
                string branchName = formatBranchName(numCarbonsInBranch, halogenType);
                int locant = i + 1;  // 1-based locant
                branches.push_back(to_string(locant) + "-" + branchName);
            }
        }
    }

    // Sort branches by locants to ensure correct order
    sort(branches.begin(), branches.end());

    // Combine branches with identical names
    vector<string> combinedBranches = combineBranches(branches);

    // Combine branch information with the main chain name
    string finalName;
    for (const string& branch : combinedBranches) {
        if (!finalName.empty()) finalName += "-";
        finalName += branch;
    }
    if (!finalName.empty()) {
        finalName += chainName;
    } else {
        finalName = chainName;
    }

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
        pair<int, vector<int>> res = dfsWithConditions(coohNode, visited, ignoredNodes);
        vector<int> path = res.second;

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
    unordered_map<int, vector<pair<int, int>>> branchInfo;

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

    // Step 2: Store branch information AND halogen information on the original chain
    for (int atom : longestChain) {
        // First, check for halogens directly on this carbon
        int graphAtomId = -1;
        string atomLabel = idToLabel[atom];
        for (const auto& pair : graph1.carbons) {
            if (pair.second.label + to_string(pair.second.id) == atomLabel) {
                graphAtomId = pair.first;
                break;
            }
        }

        if (graphAtomId != -1) {
            const CarbonNode& carbon = graph1.carbons[graphAtomId];
            
            if (carbon.C_X_bonds > 0) {
                int halogenType = 1;  // Default to chlorine for now
                
                const string& label = carbon.label;
                if (label.find("Cl") != string::npos) halogenType = 1;
                else if (label.find("Br") != string::npos) halogenType = 2;
                else if (label.find("F") != string::npos)  halogenType = 3;
                else if (label.find("I") != string::npos)  halogenType = 4;
                
                branchInfo[atom].push_back(make_pair(0, halogenType));
            }
        }
        
        // Then check for carbon branches
        for (int neighbor : graph[atom]) {
            if (ignoredNodes.find(neighbor) == ignoredNodes.end() &&
                find(longestChain.begin(), longestChain.end(), neighbor) == longestChain.end()) {
                
                // Neighbor is a branch starting point
                pair<int, int> branch = countBranchCarbons(
                    neighbor,
                    unordered_set<int>(longestChain.begin(), longestChain.end()),
                    idToLabel
                );

                // Add ALL branches (no more overwriting)
                branchInfo[atom].push_back(branch);
            }
        }
    }

    // Step 3: Decide best direction using branch info (now includes halogens)
    vector<int> optimalChain = getOptimalChainDirection(longestChain, branchInfo);

    if (hint == 1) counter = 2;

    // Print the longest carbon chain using node labels
    cout << "Longest carbon chain: ";
    for (int node : optimalChain) {
        cout << idToLabel[node] << " ";
    }
    cout << endl;

    // Step 4: Generate IUPAC name (append -oic acid if needed)
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

int main() {
    MolecularGraph graph;
    string formula;
    cout << "ENTER THE MOLECULAR FORMULA: ";
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