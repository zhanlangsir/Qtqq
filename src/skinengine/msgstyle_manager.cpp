#include "msgstyle_manager.h"

MsgStyleManager *MsgStyleManager::instance_ = NULL;

MsgStyleManager::MsgStyleManager() : 
	res_("xml"),
	curr_style_("adium") 
{
	QFile file(currStyleDir() + "/res.xml");
	file.open(QIODevice::ReadOnly);

	res_.setContent(&file);
	file.close();
}
