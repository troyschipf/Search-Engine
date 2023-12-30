#include <iostream>
#include <fstream>
#include <regex>
#include <list>
#include <string>
#include <map>
#include <set>
#include "HTML.h"

// gets the substring between two given strings used to find title, description, body, etc.
std::string getSubString(std::string input, std::string first, std::string last){
    std::size_t startPos = input.find(first);
    if(startPos == std::string::npos){
        return "";
    }
    startPos += first.length();

    std::size_t endPos = input.find(last);
    if(endPos == std::string::npos){
        return "";
    }
    return input.substr(startPos, endPos-startPos);
}

// function to parse an HTML file and extract links to local files
std::list<std::string> extractLinksFromHTML(const std::string& fileContent) {
    std::list<std::string> links;
    // regular expression to match href attributes in anchor tags
    std::regex linkRegex("<a\\s+[^>]*href\\s*=\\s*['\"]([^'\"]+)['\"][^>]*>");
    std::smatch match;

    // search for links in the HTML content
    std::string::const_iterator start = fileContent.cbegin();
    while (std::regex_search(start, fileContent.cend(), match, linkRegex)) {
        if (match.size() > 1) {
            links.push_back(match[1].str());
        }
        start = match.suffix().first;
    }

    return links;
}

// counts the amount of times the searched word occurs in the body of the document
double wordOccurence(std::string body, std::string target){
    std::stringstream inBody(body);
    std::string str;
    std::vector<std::string> strList;
    while(inBody >> str){
        strList.push_back(str);
    }
    double count = 0;
    // checks if the target contains a space denoting that it is a quote search
    bool isFound = target.find(" ") != std::string::npos;
    double wsCounter = 0;
    size_t pos =0;
    while((pos = target.find(" ", pos)) != std::string::npos){
        wsCounter++;
        pos++;
    }
    // wsCounter counts whitespace
    if(wsCounter == 1){
        if(isFound){
            for(unsigned int i=0;i<strList.size();i++){
                // checks the current and previous word
                if(i!=0 && (strList[i-1] + " " + strList[i]) == target){
                    count++;
                }
            }
        }
    }
    else if(wsCounter == 2){
        if(isFound){
            for(unsigned int i=0;i<strList.size();i++){
                // checks the current and previous 2 words
                if(i > 1 && (strList[i-2] + " " + strList[i-1] + " " + strList[i]) == target){
                    count++;
                }
            }
        }    
    }
    // if its just a single word search
    else{    
        for(unsigned int i=0;i<strList.size();i++){
            if(strList[i] == target){
                count++;
            }
        }
    }
    if(wsCounter == 1){
        return 2*count;
    }
    else if(wsCounter == 2){
        return 3*count;
    }
    else{
        return count;
    }
}

// creates the web crawler map
void webCrawler(std::string fileName, std::vector<std::string> searchWords, std::vector<HTML*>& htmlFiles, 
    std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> >& totalWordCount, bool conQuote){
    std::ifstream input(fileName);
    if(!input.is_open()){
        std::cerr << "input file could not be opened." << std::endl;
    }
    std::string aTitle;
    std::string aUrl;
    std::string aDescription;
    std::string aBody;
    std::vector<std::string> aLinks;
    HTML* htmlFile;
    double aTotalLength;
    std::vector<double> aTotalWC;
    double aWC;
    // reads the file that corresponds to the input string name and creates and HTML pointer for it
    while(input.is_open()){
        std::string fileContent((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        aTotalLength = double(fileContent.size());
        aWC = 0;
        // if not a quote search check the files for each word
        if(!conQuote){
            for(unsigned int i=0; i<searchWords.size(); i++){
                // if one of the words cant be found in the document than the document wc should be 0
                if(wordOccurence(fileContent, searchWords[i]) == 0){
                    aWC = 0;
                }
                else{
                    aWC += wordOccurence(fileContent, searchWords[i]);
                }
            }
        }
        // if it is a quote search check the files for the full quote search
        if(conQuote){
            std::string target = "";
            for(unsigned int i=0; i<searchWords.size(); i++){
                if(i+1 == searchWords.size()){
                    target += searchWords[i];
                }
                else{
                    target += (searchWords[i] + " ");
                }
            }
            aWC += wordOccurence(fileContent, target);
        }
        for(unsigned int i=0; i<searchWords.size(); i++){
            size_t pos = 0;
            double total = 0;
            while((pos = fileContent.find(searchWords[i], pos)) != std::string::npos){
                total++;
                pos += searchWords[i].size();
            }
            aTotalWC.push_back(total);
        }
        aTitle = getSubString(fileContent,"<title>", "</title>");
        aUrl = getSubString(fileContent, "<h1>", "</h1>");
        aDescription = getSubString(fileContent, "content=\"", "\">");
        aBody = getSubString(fileContent, "<body>", "</body>");
        std::list<std::string> links = extractLinksFromHTML(fileContent);
        // creates a vector of links for each HTML pointer
        for(std::list<std::string>::iterator itr = links.begin(); itr != links.end();itr++){
            int tbd = fileName.find_last_of("/");
            std::string inputName = fileName;
            inputName.erase(tbd, (inputName.size() - tbd));
            while((*itr).find("../") != std::string::npos){
                (*itr).erase(0,3);
                tbd = inputName.find_last_of("/");
                inputName.erase(tbd, (inputName.size() - tbd));
            }
            aLinks.push_back(inputName + "/" + *itr);
        }
        htmlFile = new HTML(aTitle, aUrl, aDescription, aBody, aLinks, aTotalLength, aTotalWC, aWC);
        htmlFiles.push_back(htmlFile);
        totalWordCount[htmlFile->getUrl()] = std::make_pair(std::make_pair(htmlFile->getWC(),htmlFile->getTotalWC()),
        htmlFile->getTotalLength());
        input.close();
    }   

    // recursively call the function for each link of each pointer
    // checks the totalWordCount map to see if the link has already been mapped
    for(unsigned int i=0; i<htmlFile->getLinks().size();i++){
        int counter = 0;
        for(std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> >::iterator itr=totalWordCount.begin();itr!=totalWordCount.end();itr++){
            if(htmlFile->getLinks()[i] == itr->first){
                counter++;
            }
        }
        if(counter == 0){
            webCrawler(htmlFile->getLinks()[i], searchWords, htmlFiles, totalWordCount,conQuote);
        }
    }
}

// finds and maps the total keyword density for each file
void keywordDensity(std::map<std::string,double>& densityScore, 
    std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> >& totalWordCount){
    double length = 0;
    std::vector<double> density;
    std::vector<double> keywordDensity;
    std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> >::iterator vitr = totalWordCount.begin();
    // populates the vector with 0s to be added to 
    for(unsigned int i=0; i<vitr->second.first.second.size();i++){
        density.push_back(0);
    }
    // stores all the densities of each seperate word seperately
    for(std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> >::iterator itr = totalWordCount.begin(); 
        itr!=totalWordCount.end(); itr++){
        length += itr->second.second;
        for(unsigned int i=0; i<itr->second.first.second.size();i++){
            density[i] += (itr->second.first.second[i]);
        }
    }
    // populates the vector with 0s to be added to 
    for(unsigned int i=0; i<density.size(); i++){
        keywordDensity.push_back(0);
    }
    for(unsigned int i=0;i<keywordDensity.size();i++){
        keywordDensity[i] += (density[i] / length);
    }
    for(std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> >::iterator itr = totalWordCount.begin(); 
        itr!=totalWordCount.end(); itr++){
        // checks if the word occurs in the body of this file
        if(itr->second.first.first > 0){
            std::vector<double> kwdScores;
            // iterates through the word occurrences in the vector of total word occurences
            for(unsigned int i=0; i<itr->second.first.second.size();i++){
                kwdScores.push_back( itr->second.first.second[i] / (itr->second.second * keywordDensity[i]));
            }
            double kwdScore = 0;
            for(unsigned int i=0; i<kwdScores.size();i++){
                kwdScore += kwdScores[i];
            }
            if(kwdScore != 0){
                densityScore[itr->first] = kwdScore;
            }
        }   
    }
}

// creates a map that stores the url and the backlinks scor
void backlinksScoreMap(std::vector<HTML*>& htmlFiles, std::map<std::string, double> densityScore,
    std::map<std::string, double>& backlinksMap){
    // iterate through the densityScore map and find all html files that contain links for the current URL
    for(std::map<std::string, double>::iterator itr = densityScore.begin(); itr!=densityScore.end();itr++){
        double blscore = 0;
        for(unsigned int j=0; j<htmlFiles.size(); j++){
            double size = htmlFiles[j]->getLinks().size();
            for(unsigned int k=0; k<size; k++){
                if(itr->first == htmlFiles[j]->getLinks()[k]){
                    blscore += (1/(1 + size));
                }
            }
        }
        backlinksMap[itr->first] = blscore;
    }
}

// calculates the overall page score and maps it
void keywordDensityScore(std::map<std::string, double> backlinksMap, std::map<std::string, double> densityScore, 
    std::map<double,std::string>& kwDensMap, std::vector<HTML*> htmlFiles){
    std::map<std::string, double>::iterator densItr = densityScore.begin();
    std::map<std::string, double>::iterator blItr = backlinksMap.begin();
    double kwdScore;
    while(densItr!=densityScore.end() && blItr != backlinksMap.end()){
        if(densItr->second != 0){
            kwdScore = ((0.5 * densItr->second) + (0.5 * blItr->second));
            kwDensMap[kwdScore] = densItr->first;
        }
        blItr++;
        densItr++;
        kwdScore=0;
    }
}

// creates the snippet for the output 
std::string createSnippet(HTML* htmlFile, std::vector<std::string> searchWords){
    std::string fullQuery = "";
    std::string snippet = "";
    for(unsigned int i=0;i<searchWords.size();i++){
        if(i+1 == searchWords.size()){
            fullQuery += searchWords[i];
        }
        else{ 
            fullQuery += (searchWords[i] + " ");
        }
    }
    size_t queryPos = 0;
    if(htmlFile->getBody().find(fullQuery) != std::string::npos){
        queryPos = htmlFile->getBody().find(fullQuery);
        // gives position of period before query
        size_t periodPos = htmlFile->getBody().rfind(".", queryPos);
        size_t startPos = periodPos + 1;
        // checks for the proper starting point of snippet
        while(std::isspace(htmlFile->getBody()[startPos])){
            startPos++;
        }
        size_t finalPos = startPos + 120;
        for(size_t i = startPos; i<finalPos;i++){
            snippet += htmlFile->getBody()[i];
        }
    }else if(htmlFile->getBody().find(searchWords[0]) != std::string::npos){
        queryPos = htmlFile->getBody().find(searchWords[0]);
        // gives position of period before query
        size_t periodPos = htmlFile->getBody().rfind(".", queryPos);
        size_t startPos = periodPos + 1;
        // checks for the proper starting point of snippet
        while(std::isspace(htmlFile->getBody()[startPos])){
            startPos++;
        }
        size_t finalPos = startPos + 120;
        for(size_t i = startPos; i<finalPos;i++){
            snippet += htmlFile->getBody()[i];
        }
    }
    return snippet;
}

// prints all the proper information to the output file
void printDocs(std::vector<HTML*> htmlFiles, std::map<double, std::string> kwDensMap, std::ofstream& output,
    std::vector<std::string> searchWords){
    output << "Matching documents: " << std::endl;
    for(std::map<double, std::string>::reverse_iterator itr = kwDensMap.rbegin(); itr!=kwDensMap.rend(); itr++){
        output << std::endl;
        for(unsigned int i=0; i<htmlFiles.size(); i++){
            if(htmlFiles[i]->getUrl() == itr->second){
                output << "Title: " << htmlFiles[i]->getTitle() << std::endl;
                output << "URL: " << htmlFiles[i]->getUrl() << std::endl;
                output << "Description: " << htmlFiles[i]->getDescription() << std::endl;
                std::string snippet = createSnippet(htmlFiles[i], searchWords);
                output << "Snippet: " << snippet << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]){
    std::string fileName = argv[1];
    std::ofstream output(argv[2]);
    std::vector<std::string> searchWords;
    std::string query = argv[3];
    size_t quotePos;
    bool conQuote = false;
    for(int i=3;i<argc;i++){
        query = argv[i];
        // remove the double quote character at position
        if( (quotePos = query.find('"')) != std::string::npos ){
            query.erase(quotePos, 1); 
            conQuote = true;
        }
        searchWords.push_back(query);
    }
    if(argc > 6){
        std::cerr << "too many input arguments" << std::endl;
        return 1;
    }
    if(!output.is_open()){
        std::cerr << "output file was not opened properly." << std::endl;
        return 1;
    }  

    // calling all the functions and creating all the maps 
    std::map<std::string,std::pair<std::pair<double,std::vector<double>>,double> > totalWordCount;
    std::vector<HTML*> htmlFiles; 
    webCrawler(fileName, searchWords, htmlFiles, totalWordCount,conQuote);
    std::map<std::string, double> densityScore;
    keywordDensity(densityScore, totalWordCount);
    std::map<std::string,double> backlinksMap;
    backlinksScoreMap(htmlFiles, densityScore, backlinksMap);
    std::map<double,std::string> kwDensMap;
    keywordDensityScore(backlinksMap,densityScore,kwDensMap, htmlFiles);

    // checks if there are any matching documents
    if(kwDensMap.size() > 0){
        printDocs(htmlFiles,kwDensMap,output,searchWords);
    }else{
        std::string searchWord = "";
        for(unsigned int i=0; i<searchWords.size();i++){
            if(i+1 == searchWords.size()){
                searchWord += searchWords[i];
            }
            else{
                searchWord += (searchWords[i] + " ");
            }
        }
        output << "Your search - " << searchWord << " - did not match any documents." << std::endl;
    }
    return 0;
}