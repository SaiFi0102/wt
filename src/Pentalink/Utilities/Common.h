#ifndef TP_COMMON_H
#define TP_COMMON_H

#define SET_TRANSIENT_VIEW_DIALOG(dialog) dialog->setClosable(true);\
	dialog->resize(Wt::WLength(85, Wt::WLength::Percentage), Wt::WLength(95, Wt::WLength::Percentage));\
	dialog->setTransient(true);\
	dialog->setDeleteWhenHidden(true);\
	dialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

namespace PL
{
	
}

#endif 