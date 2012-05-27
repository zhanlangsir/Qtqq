#ifndef QTQQ_GROUPIMGLOADER_H
#define QTQQ_GROUPIMGLOADER_H

#include "imgloader.h"

class GroupImgLoader : public ImgLoader
{
public:
     GroupImgLoader();

protected:
    QByteArray getImgUrl(const LoadInfo &info) const;
};

#endif // QTQQ_GROUPIMGLOADER_H
