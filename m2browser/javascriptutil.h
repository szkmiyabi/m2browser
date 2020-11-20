#ifndef JAVASCRIPTUTIL_H
#define JAVASCRIPTUTIL_H

#include <QString>

class JavascriptUtil
{
public:

    //コンストラクタ
    JavascriptUtil();

    //CSSカット検証
    static QString css_cut();
    //alt属性値検証
    static QString image_alt();
    //target属性値検証
    static QString target_attr();
    //ページ構造検証
    static QString semantic_check();
    //lang属性検証
    static QString lang_attr();
    //フォームラベルとtitle属性検証
    static QString tag_label_and_title_attr();
    //ドキュメントリンク検証
    static QString document_link();
    //フォーカス検証
    static QString super_focus();
    //WAI-ARIA属性検証
    static QString wai_aria_attr();
};

#endif // JAVASCRIPTUTIL_H
