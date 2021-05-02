//
// Created by weiwu on 21-5-2.
//

#ifndef RIBBONFU_FRAMELESSHELPER_H
#define RIBBONFU_FRAMELESSHELPER_H


#include <QtCore/QObject>

class FramelessHelperPrivate;

class FramelessHelper : public QObject{
    Q_OBJECT
public:
    explicit FramelessHelper(QObject *parent);
    ~FramelessHelper();

    void activateOn(QWidget *topLevelWidget);
    void removeFrom(QWidget *topLevelWidget);
    void setWidgetMovable(bool movable);
    void setWidgetResizable(bool resizable);
    void setRubberBandOnMove(bool movable);
    void setRubberBandOnResize(bool resizable);

    void setBorderWidth(uint width);
    void setTitleHeight(uint height);
    bool widgetResizable();
    bool widgetMovable();
    bool rubberBandOnMove();
    bool rubberBandOnResisze();
    uint borderWidth();
    uint titleHeight();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    FramelessHelperPrivate *m_d;
};


#endif //RIBBONFU_FRAMELESSHELPER_H
