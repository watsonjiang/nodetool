#ifndef XML_PATH_H
#define XML_PATH_H

#include <string>
#include <string.h>
#include <stdarg.h>
#include "tinyxml.h"

/*!
 * @class XmlPath 
 * @brief read xml with xpath syntax
 */
class XmlPath {
public:
	explicit XmlPath(TiXmlElement *top, const char *name): top_(top), name_(name) {}

        ~XmlPath() {}

	XmlPath getElement(const char *path, bool required = true) const;

	const char *getString(const char *path, bool required = true) const;

	const char *getString() const;

	long long getNumber(const char *path, bool required = true) const ;

	void next();

	bool valid() const;

private:
	TiXmlElement *top_;
	std::string name_;
};      //XmlPath

class XmlDocument {
public:
	XmlDocument() {}

	~XmlDocument() {}

	XmlPath loadFile(const char *path, const char *top);
private:
	TiXmlDocument doc_;
};

#endif	//XML_PATH_H
