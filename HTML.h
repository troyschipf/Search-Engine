#ifndef HTML_H_
#define HTML_H_
#include <iostream>
#include <vector>
class HTML{
	public:
		HTML(std::string& aTitle, std::string& aUrl, std::string& aDescription, std::string& aBody, 
			std::vector<std::string>& aLinks, double& aTotalLength, std::vector<double>& aTotalWC, double& aWC);

		const std::string& getTitle() const{return title;}
		const std::string& getUrl() const{return url;}
		const std::string& getDescription() const{return description;}
		const std::string& getBody() const{return body;}
		const std::vector<std::string>& getLinks() const{return links;}
		const double& getTotalLength() const{return totalLength;}
		const std::vector<double> getTotalWC() const{return totalWC;}
		const double& getWC() const{return WC;}

	private:
		std::string title;
		std::string url;
		std::string description;
		std::string body;
		std::vector<std::string> links;
		double totalLength;
		std::vector<double> totalWC;
		double WC;

};
#endif