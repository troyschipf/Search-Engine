#include "HTML.h"
#include <iostream>

HTML::HTML(std::string& aTitle, std::string& aUrl, std::string& aDescription, std::string& aBody, 
	std::vector<std::string>& aLinks, double& aTotalLength, std::vector<double>& aTotalWC, double& aWC){
	title = aTitle;
	url = aUrl;
	body = aBody;
	description = aDescription;
	links = aLinks;
	totalLength = aTotalLength;
	totalWC = aTotalWC;
	WC = aWC;
}
