#include "javascriptutil.h"

JavascriptUtil::JavascriptUtil()
{}


//CSSカット検証
QString JavascriptUtil::css_cut()
{
    QString jsc = QString();
    jsc.append("var d = document;");
    jsc.append("var delarr = new Array();");
    jsc.append("var links = d.getElementsByTagName(\"link\");");
    jsc.append("for(var i=0; i<links.length; i++) {");
    jsc.append("var link = links.item(i);");
    jsc.append("var href = link.getAttribute(\"href\");");
    jsc.append("if(is_css_file(href) || is_css_link(link)) {");
    jsc.append("delarr.push(href);");
    jsc.append("}");
    jsc.append("}");
    jsc.append("for(var i=0; i<delarr.length; i++) {");
    jsc.append("var line = delarr[i];");
    jsc.append("delete_link(line);");
    jsc.append("}");
    jsc.append("var tags = d.getElementsByTagName(\"*\");");
    jsc.append("for(var i=0; i<tags.length; i++) {");
    jsc.append("var tag = tags.item(i);");
    jsc.append("var style = tag.getAttribute(\"style\");");
    jsc.append("if(style !== null || style !== \"\") {");
    jsc.append("tag.removeAttribute(\"style\");");
    jsc.append("}");
    jsc.append("}");
    jsc.append("var styles = d.getElementsByTagName(\"style\");");
    jsc.append("for(i=0; i<styles.length; i++) {");
    jsc.append("var style = styles.item(i);");
    jsc.append("style.textContent = null;");
    jsc.append("}");
    jsc.append("function is_css_file(href) {");
    jsc.append("var pat = new RegExp(\".+\\.css\");");
    jsc.append("if(pat.test(href)) return true;");
    jsc.append("else return false;");
    jsc.append("}");
    jsc.append("function is_css_link(link) {");
    jsc.append("if(link.hasAttribute(\"type\")) {");
    jsc.append("var pat = new RegExp(\"text/css\");");
    jsc.append("var type = link.getAttribute(\"type\");");
    jsc.append("if(pat.test(type)) return true;");
    jsc.append("else return false;");
    jsc.append("} else {");
    jsc.append("return false;");
    jsc.append("}");
    jsc.append("}");
    jsc.append("function delete_link(line) {");
    jsc.append("var lks = d.getElementsByTagName(\"link\");");
    jsc.append("for(var j=0; j<lks.length; j++) {");
    jsc.append("var lk = lks.item(j);");
    jsc.append("var hf = lk.getAttribute(\"href\");");
    jsc.append("if(hf === line) {");
    jsc.append("lk.parentNode.removeChild(lk);");
    jsc.append("break;");
    jsc.append("}");
    jsc.append("}");
    jsc.append("}");
    return jsc;

}
