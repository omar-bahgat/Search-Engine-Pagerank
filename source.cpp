/*
 Name: Omar Ashraf Bahgat
 Course: Analysis and Design of Algorithms Lab (Fall 2022)
 Project: Search Engine using Google's PageRank algorithm
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <iterator>
using namespace std;

// all data structures used

unordered_map<string,int> WebToIndex;                      // map website name to an index
unordered_map<int,string> IndextoWeb;                      // retrieve website name from its index
unordered_map<int,vector<int>> adjlist;                    // webgraph (directed)
unordered_map<int,vector<int>> transpose;                  // webgraph transpose
unordered_map<string, vector<int>> keywords;               // map keyword to websites indices its in
unordered_map<int, int> impressions;                       // map website index to its number of impressions
unordered_map<int, int> clicks;                            // map website index to its number of clicks
vector<double> PageRank;                                   // stores PageRank of all website indices (1-indexed)

vector<int> Intersection(vector<int> &, vector<int> &);

void ConstructWebgraph();
void LoadKeywords();
void LoadImpressions();
void LoadClicks();

vector<int> searchQuotations(string);
vector<int> searchAND(string, string);
vector<int> searchOR(string, string);
vector<int> searchDefault(vector<string>);
vector<int> search(string);

void InitializePageRank();

void exit();
double getScore(int i);
void NewSearch();
void ViewResults(vector<int>);
void ViewWebsite(vector<int>, int);
void SecondUserInteraction(vector<int>);
void InitialUserInteraction();
void InitialText();

int idx = 1; // used to uniquely assign indices to websites

vector<int> Intersection(vector<int> &a, vector<int> &b){   // find intersection between 2 sorted vectors
    vector<int> answer;
    int i = 0, j = 0;

    while (i < a.size() && j < b.size()){
        if (a[i] == b[j]) {
            answer.push_back(a[i]);
            i++; j++;
        }
        else if (a[i] < b[j]) i++;
        else j++;
    }
    return answer;
}

// loading files

void ConstructWebgraph(){
    ifstream WebgraphInput;
    WebgraphInput.open("webgraph.csv");
    
    if(!WebgraphInput.is_open()) cout << "Error in opening \"webgraph.csv\"\n";
    
    string line, src, dest;
    
    while (getline(WebgraphInput, line)){
        stringstream str(line);
            
        getline(str, src, ',');
        getline(str, dest, '*');    // * is the delimiting character
        
        if(WebToIndex[src] == 0){       // mapping source to an index
            WebToIndex[src] = idx;
            IndextoWeb[idx] = src;
            idx++;
        }
        
        if(WebToIndex[dest] == 0){      // mapping destination to an index
            WebToIndex[dest] = idx;
            IndextoWeb[idx] = dest;
            idx++;
        }
        
        adjlist[WebToIndex[src]].push_back(WebToIndex[dest]);       // add edge from src to dest in webgraph
        transpose[WebToIndex[dest]].push_back(WebToIndex[src]);     // add edge from dest to src in transpose

    }
    
    WebgraphInput.close();
}

void LoadKeywords(){
    ifstream KeywordsInput;
    KeywordsInput.open("keywords.csv");
    
    if(!KeywordsInput.is_open()) cout << "Error in opening \"keywords.csv\"\n";

    string line, website, word;
    
    while (getline(KeywordsInput, line)){
        stringstream str(line);
        getline(str, website, ',');
        
        while (getline(str, word, ',')){
                keywords[word].push_back(WebToIndex[website]);  // push each website to its corresponding keyword
            }
        }

    KeywordsInput.close();
}

void LoadImpressions(){
    ifstream ImpressionsInput;
    ImpressionsInput.open("impressions.csv");
        
    if(!ImpressionsInput.is_open()) cout << "Error in opening \"impressions.csv\"\n";

    string line, website, impression;

    while(getline(ImpressionsInput, line)){
        stringstream str(line);
        
        getline(str, website, ',');
        getline(str, impression, '*');
        impressions[WebToIndex[website]] = stoi(impression);    // initialize impressions for website using its index
    }
    
    ImpressionsInput.close();
}

void LoadClicks(){
    ifstream ClicksInput;
    ClicksInput.open("clicks.csv");
    
    if(!ClicksInput.is_open()) cout << "Error in opening \"clicks.csv\"\n";
    
    string line, website, click;
    
    while(getline(ClicksInput,line)){
        stringstream str(line);
        
        getline(str, website, ',');
        getline(str, click, '*');
        clicks[WebToIndex[website]] = stoi(click);    // initialize clicks for website using its index
    }
    
    ClicksInput.close();
}

void InitializePageRank(){
    const int N = idx-2;              // number of nodes in webgraph (idx increases by 2 when reading files so I decrement it here)
    
    vector<double> prev(N+1,1.0/N);   // default value for all websites (1/N)
    PageRank.resize(N+1,0.0);           // 1-indexing
    
    vector<int> :: iterator it;
    double MAX = -1e9;
    
    for(int i = 1; i <= 100; i++){      // 100 iterations gurantees good results according to many websites
        for(int j = 1; j <= N; j++){    // for each node, check all nodes pointing to it
            for(it = transpose[j].begin(); it != transpose[j].end(); it++){
                PageRank[j] += prev[*it] / adjlist[*it].size();              // pagerank / # of outgoing links
            }
            MAX = max(MAX,PageRank[j]);
        }
        if(i == 100) break;     // to prevent resetting PageRank before exiting loop
        prev = PageRank;
        PageRank.resize(N+1,0.0);
    }
    
    // normalizing
    for(int i = 1; i <= N; i++){
        PageRank[i] /= MAX;
    }
}

// searching process

vector<int> searchQuotations(string statement){
    vector<int> answer;
    statement = statement.substr(1,statement.size()-2); // remove quotations
    
    vector<int> :: iterator it;
    for(it = keywords[statement].begin(); it != keywords[statement].end(); it++){
        answer.push_back(*it);
        impressions[*it]++;         // increase impressions for each website that has required keyword
    }
    
    return answer;
}

vector<int> searchAND(string word1, string word2){
    vector<int> answer, a, b;
    
    vector<int> :: iterator it;
    for(it = keywords[word1].begin(); it != keywords[word1].end(); it++){
        a.push_back(*it);
    }
    for(it = keywords[word2].begin(); it != keywords[word2].end(); it++){
        b.push_back(*it);
    }
    
    answer = Intersection(a,b);         // find websites that have both keywords using intersection function
    
    for(int i = 0 ; i < answer.size(); i++){
        impressions[answer[i]]++;
    }
    
    return answer;
}

vector<int> searchOR(string word1, string word2){
    vector<int> answer;
    set<int> s;
    
    vector<int> :: iterator it;
    for(it = keywords[word1].begin(); it != keywords[word1].end(); it++){
        s.insert(*it);
    }
    for(it = keywords[word2].begin(); it != keywords[word2].end(); it++){
        s.insert(*it);
    }
    
    set<int> :: iterator its;
    for(its = s.begin(); its != s.end(); its++){
        answer.push_back(*its);
        impressions[*its]++;
    }
    
    return answer;
}

vector<int> searchDefault(vector<string> words){
    vector<int> answer;
    set<int> s;
    
    vector<int> :: iterator it;
    for(int i = 0; i < words.size(); i++){
        for(it = keywords[words[i]].begin(); it != keywords[words[i]].end(); it++){
            s.insert(*it);
        }
    }

    set<int> :: iterator its;
    for(its = s.begin(); its != s.end(); its++){
        answer.push_back(*its);
        impressions[*its]++;
    }
    
    return answer;
}

vector<int> search(string query){
    vector<int> answer;         // vector with websites that contain the required keywords
    
    if(query.front() == '"' && query.back() == '"'){
        answer = searchQuotations(query);
    }
    else{
        vector<string> words;
        string temp = "";
        
        for(int i = 0; i < query.size(); i++){
            if(query[i] == ' '){
                words.push_back(temp);
                temp = "";
            }
            else{
                temp += query[i];
            }
        }
        words.push_back(temp);
 
        if(words[1] == "AND" && words.size() == 3){
            answer = searchAND(words.front(), words.back());
        }
        else if (words[1] == "OR" && words.size() == 3){
            answer = searchOR(words.front(), words.back());
        }
        else{
            answer = searchDefault(words);
        }
    }
    return answer;
}

double getScore(int i){
    
    double fraction = (0.1 * impressions[i]) / (1.0 + 0.1 * impressions[i]);
    
    double score = 0.4 * PageRank[i] + ((1.0 - fraction) * PageRank[i] + fraction * (clicks[i] / impressions[i])) * 0.6;
    
    return score;
    
}

// updating files before exiting the program so the updates won't be lost when starting the program again

void exit(){
    
    // updating impressions file
    
    ofstream ImpressionsOutput;
    ImpressionsOutput.open("impressions.csv");
    
    if(!ImpressionsOutput.is_open()) cout << "Error in opening \"impressions.csv\"\n";
    
    for(int i = 1; i <= idx - 2; i++){
        ImpressionsOutput << IndextoWeb[i] << "," << impressions[i] << "*" << "\n";
    }
    
    ImpressionsOutput.close();
    
    // updating clicks file
    
    ofstream ClicksOutput;
    ClicksOutput.open("clicks.csv");
    
    if(!ClicksOutput.is_open()) cout << "Error in opening \"clicks.csv\"\n";

    
    for(int i = 1; i <= idx - 2; i++){
        ClicksOutput << IndextoWeb[i] << "," << clicks[i] << "*" << "\n";
    }
    
    ClicksOutput.close();

}

void ViewResults(vector<int> answer){
    
    cout << "\nSearch results: \n";
    if(answer.size() == 0) cout << "No results found\n";
    
    else{
        for(int i = 0; i < answer.size(); i++){
            cout << i+1 << ". " << IndextoWeb[answer[i]] << "\n";
        }
    }
}

void NewSearch(){
    
    cout << "Enter query: ";
    string query;
    cin.ignore();                         // ignore leading whitespace
    getline(cin, query,'\n');
    
    
    vector<int> answer = search(query);
        
    vector<pair<double,int>> temp_answer(answer.size());
    
    for(int i = 0; i < answer.size(); i++){
        temp_answer[i].first = getScore(answer[i]);
        temp_answer[i].second = answer[i];
    }
    sort(temp_answer.rbegin(),temp_answer.rend()); // descending order
    
    for(int i = 0; i < answer.size(); i++){
        answer[i] = temp_answer[i].second;
    }
    
    ViewResults(answer);
    
    SecondUserInteraction(answer);
}

void ViewWebsite(vector<int> answer, int index){
    
    clicks[index]++;        // updating clicks for clicked websites
    cout << "\nYou're now viewing \"" << IndextoWeb[index] << "\"\n\n";
    
    cout << "Would you like to:\n";
    cout << "1. Go back to search results\n";
    cout << "2. New search\n";
    cout << "3. Exit\n\n";
    
    int option;
    
    alternate:
    cout << "Type in your choice: ";
    cin >> option;
    
    if(option == 1){
        ViewResults(answer);
        SecondUserInteraction(answer);
    }
    else if(option == 2){
        NewSearch();
    }
    else if (option == 3){
        exit();
    }
    else if (cin.fail()){                       // prevent an infinite loop if a char or string is inputted instead of int
        cin.clear(); cin.ignore(512, '\n');
        cout << "\n-- Enter a digit --\n\n";
        goto alternate;
    }
    else{
        cout << "\n-- Invalid Choice --\n\n";
        goto alternate;
    }
    
}

void SecondUserInteraction(vector<int> answer){
    
    cout << "\nWould you like to:\n";
    cout << "1. Choose a webpage to open\n";
    cout << "2. New search\n";
    cout << "3. Exit\n\n";
    
    alternate:
    cout << "Type in your choice: ";
    int option;
    cin >> option;
    
    if(option == 1){
        cout << "Type webpage number: ";
        int option;
        cin >> option;
        ViewWebsite(answer,answer[option-1]);
    }
    else if(option == 2){
        NewSearch();
    }
    else if(option == 3){
        exit();
    }
    else if (cin.fail()){
        cin.clear(); cin.ignore(512, '\n');
        cout << "\n-- Enter a digit --\n\n";
        goto alternate;
    }
    else{
        cout << "\n-- Invalid Choice --\n\n";
        goto alternate;
    }
}

void InitialUserInteraction(){

    cout << "Type in your choice: ";
    int option;
    cin >> option;
    
    if(option == 1){
        
        cout << "\nValid queries can be any of the following:\n";
        cout << "1. Sentence or word enclosed by double quotations \" \" \n";
        cout << "2. Two words separated by AND\n";
        cout << "3. Two Words separated by OR\n";
        cout << "4. Plain text \n\n";
        NewSearch();
    }
    
    else if (option == 2){
        exit();
    }
    else if (cin.fail()){
        cin.clear(); cin.ignore(512, '\n');
        cout << "\n-- Enter a digit --\n\n";
        InitialUserInteraction();
    }
    else{
        cout << "\n-- Invalid Choice --\n\n";
        InitialUserInteraction();
    }
}

void InitialText(){
    
    cout << "Welcome!\n\n";
    cout << "What would you like to do?\n";
    cout << "1. New search\n";
    cout << "2. Exit\n\n";
    
    InitialUserInteraction();
}

int main(){
    
    ConstructWebgraph();
    LoadKeywords();
    LoadImpressions();
    LoadClicks();

    InitializePageRank();
    
    InitialText();
    
}





