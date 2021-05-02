//
// Created by weiwu on 21-5-2.
//

#include <QtCore/QHash>
#include <QRect>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QRubberBand>
#include "FramelessHelper.h"

class WidgetData;

class FramelessHelperPrivate {
public:
    QHash<QWidget *, WidgetData *> m_widgetDataHash;
    bool m_bWidgetMovable : true;
    bool m_bWidgetResizable : true;
    bool m_bRubberBandOnResize : true;
    bool m_bRubberBandOnMove : true;
};

class CursorPosCalculator {
public:
    explicit CursorPosCalculator();

    void reset();

    void recalculate(const QPoint &globalMousePos, const QRect &frameRect);

public:
    bool m_bOnEdges : true;
    bool m_bOnLeftEdge : true;
    bool m_bOnRightEdge : true;
    bool m_bOnTopEdge : true;
    bool m_bOnBottomEdge : true;
    bool m_bOnTopLeftEdge : true;
    bool m_bOnBottomLeftEdge : true;
    bool m_bOnTopRightEdge : true;
    bool m_bOnBottomRightEdge : true;

    static int m_nBorderWidth;
    static int m_nTitleHeight;
};

int CursorPosCalculator::m_nBorderWidth = 5;
int CursorPosCalculator::m_nTitleHeight = 30;

/***** CursorPosCalculator *****/
CursorPosCalculator::CursorPosCalculator() {
    reset();
}

void CursorPosCalculator::reset() {
    m_bOnEdges = false;
    m_bOnLeftEdge = false;
    m_bOnRightEdge = false;
    m_bOnTopEdge = false;
    m_bOnBottomEdge = false;
    m_bOnTopLeftEdge = false;
    m_bOnBottomLeftEdge = false;
    m_bOnTopRightEdge = false;
    m_bOnBottomRightEdge = false;
}

void CursorPosCalculator::recalculate(const QPoint &globalMousePos, const QRect &frameRect) {
    int globalMouseX = globalMousePos.x();
    int globalMouseY = globalMousePos.y();

    int frameX = frameRect.x();
    int frameY = frameRect.y();

    int frameWidth = frameRect.width();
    int frameHeight = frameRect.height();

    m_bOnLeftEdge = (globalMouseX >= frameX &&
                     globalMouseX <= frameX + m_nBorderWidth);

    m_bOnRightEdge = (globalMouseX >= frameX + frameWidth - m_nBorderWidth &&
                      globalMouseX <= frameX + frameHeight);

    m_bOnTopEdge = (globalMouseY >= frameY &&
                    globalMouseY <= frameY + m_nBorderWidth);

    m_bOnBottomEdge = (globalMouseY >= frameY + frameHeight - m_nBorderWidth &&
                       globalMouseY <= frameY + frameHeight);

    m_bOnTopLeftEdge = m_bOnTopEdge && m_bOnLeftEdge;
    m_bOnBottomLeftEdge = m_bOnBottomEdge && m_bOnLeftEdge;
    m_bOnTopRightEdge = m_bOnTopEdge && m_bOnRightEdge;
    m_bOnBottomRightEdge = m_bOnBottomEdge && m_bOnRightEdge;

    m_bOnEdges = m_bOnLeftEdge || m_bOnRightEdge || m_bOnTopEdge || m_bOnBottomEdge;
}

/**
 * WidgetData
 * update mouse style, move frame, resize frame
 */

class WidgetData {
public:
    explicit WidgetData(FramelessHelperPrivate *d, QWidget *pTopLevelWidget);

    ~WidgetData();

    QWidget *widget();

    bool handleWidgetEvent(QEvent *event);

    void updateRubberBandStatus();

private:
    void updateCursorShape(const QPoint &gMousePos);

    void resizeWidget(const QPoint &gMousePos);

    void moveWidget(const QPoint &gMousePos);

    bool handleMousePressEvent(QMouseEvent *event);

    bool handleMouseReleaseEvent(QMouseEvent *event);

    bool handleMouseMoveEvent(QMouseEvent *event);

    bool handleLeaveEvent(QEvent *event);

    bool handleHoverMoveEvent(QHoverEvent *event);

    bool handleDoubleClickedMouseEvent(QMouseEvent *event);

private:
    FramelessHelperPrivate *d;
    QRubberBand *m_pRubberBand;
    QWidget *m_pWidget;
    QPoint m_ptDragPos;
    CursorPosCalculator m_pressedMousePos;
    CursorPosCalculator m_moveMousePos;
    bool m_bLeftButtonPressed;
    bool m_bCursorShapeChanged;
    bool m_bLeftButtonTitlePressed;
    Qt::WindowFlags m_windowFlags;
};

WidgetData::WidgetData(FramelessHelperPrivate *_d, QWidget *pTopLevelWidget) {
    d = _d;
    m_pWidget = pTopLevelWidget;
    m_bLeftButtonPressed = false;
    m_bCursorShapeChanged = false;
    m_bLeftButtonTitlePressed = false;
    m_pRubberBand = NULL;

    m_windowFlags = m_pWidget->windowFlags();
    m_pWidget->setMouseTracking(true);
    m_pWidget->setAttribute(Qt::WA_Hover, true);

    updateRubberBandStatus();
}

WidgetData::~WidgetData() {
    m_pWidget->setMouseTracking(false);
    m_pWidget->setWindowFlags(m_windowFlags);
    m_pWidget->setAttribute(Qt::WA_Hover, false);

    delete m_pRubberBand;
    m_pRubberBand = NULL;
}

QWidget *WidgetData::widget() {
    return (m_pWidget);
}

bool WidgetData::handleWidgetEvent(QEvent *event) {
    switch (event->type()) {
        case QEvent::MouseButtonPress:
            return (handleMousePressEvent(static_cast<QMouseEvent *>(event)));

        case QEvent::MouseButtonRelease:
            return (handleMouseReleaseEvent(static_cast<QMouseEvent *>(event)));
        case QEvent::MouseMove:
            return (handleMouseMoveEvent(static_cast<QMouseEvent *>(event)));

        case QEvent::Leave:
            return (handleLeaveEvent(static_cast<QMouseEvent *>(event)));

        case QEvent::HoverMove:
            return (handleHoverMoveEvent(static_cast<QHoverEvent *>(event)));

        case QEvent::MouseButtonDblClick:
            return (handleDoubleClickedMouseEvent(static_cast<QMouseEvent *>(event)));

        default:
            break;
    }
    return (false);
}

void WidgetData::updateRubberBandStatus() {
    if (d->m_bRubberBandOnMove || d->m_bRubberBandOnResize) {
        if (NULL == m_pRubberBand) {
            m_pRubberBand = new QRubberBand(QRubberBand::Rectangle);
        } else {
            delete m_pRubberBand;
            m_pRubberBand = NULL;
        }
    }
}

void WidgetData::updateCursorShape(const QPoint &gMousePos) {
    if (m_pWidget->isFullScreen() || m_pWidget->isMaximized()) {
        if (m_bCursorShapeChanged) {
            m_pWidget->unsetCursor();
        }
        return;
    }

    m_moveMousePos.recalculate(gMousePos, m_pWidget->frameGeometry());

    if (m_moveMousePos.m_bOnTopLeftEdge || m_moveMousePos.m_bOnBottomRightEdge) {
        m_pWidget->setCursor(Qt::SizeFDiagCursor);
        m_bCursorShapeChanged = true;
    } else if (m_moveMousePos.m_bOnTopRightEdge || m_moveMousePos.m_bOnBottomLeftEdge) {
        m_pWidget->setCursor(Qt::SizeBDiagCursor);
        m_bCursorShapeChanged = true;
    } else if (m_moveMousePos.m_bOnLeftEdge || m_moveMousePos.m_bOnRightEdge) {
        m_pWidget->setCursor(Qt::SizeHorCursor);
        m_bCursorShapeChanged = true;
    } else if (m_moveMousePos.m_bOnTopEdge || m_moveMousePos.m_bOnBottomEdge) {
        m_pWidget->setCursor(Qt::SizeVerCursor);
        m_bCursorShapeChanged = true;
    } else {
        if (m_bCursorShapeChanged) {
            m_pWidget->unsetCursor();
            m_bCursorShapeChanged = false;
        }
    }
}

void WidgetData::resizeWidget(const QPoint &gMousePos) {
    QRect origRect;

    if (d->m_bRubberBandOnResize) {
        origRect = m_pRubberBand->frameGeometry();
    } else {
        origRect = m_pWidget->frameGeometry();
    }

    int left = origRect.left();
    int top = origRect.top();
    int right = origRect.right();
    int bottom = origRect.bottom();

    origRect.getCoords(&left, &top, &right, &bottom);

    int minWidth = m_pWidget->minimumWidth();
    int minHeight = m_pWidget->minimumHeight();

    if (m_pressedMousePos.m_bOnTopLeftEdge) {
        left = gMousePos.x();
        top = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnBottomLeftEdge) {
        left = gMousePos.x();
        bottom = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnTopRightEdge) {
        right = gMousePos.x();
        top = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnBottomRightEdge) {
        right = gMousePos.x();
        bottom = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnLeftEdge) {
        left = gMousePos.x();
    } else if (m_pressedMousePos.m_bOnRightEdge) {
        right = gMousePos.x();
    } else if (m_pressedMousePos.m_bOnTopEdge) {
        top = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnBottomEdge) {
        bottom = gMousePos.y();
    }

    QRect newRect(QPoint(left, top), QPoint(right, bottom));

    if (newRect.isValid()) {
        if (minWidth > newRect.width()) {
            if (left != origRect.left()) {
                newRect.setLeft(origRect.left());
            } else {
                newRect.setRight(origRect.right());
            }
        }
        if (minHeight > newRect.height()) {
            if (top != origRect.top()) {
                newRect.setTop(origRect.top());
            } else {
                newRect.setBottom(origRect.bottom());
            }
        }

        if (d->m_bRubberBandOnResize) {
            m_pRubberBand->setGeometry(newRect);
        } else {
            m_pWidget->setGeometry(newRect);
        }
    }
}


void WidgetData::moveWidget(const QPoint &gMousePos) {
    if (d->m_bRubberBandOnMove) {
        m_pRubberBand->move(gMousePos - m_ptDragPos);
    } else {
        m_pWidget->move(gMousePos - m_ptDragPos);
    }
}


bool WidgetData::handleMousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_bLeftButtonPressed = true;
        m_bLeftButtonTitlePressed = event->pos().y() < m_moveMousePos.m_nTitleHeight;

        QRect frameRect = m_pWidget->frameGeometry();
        m_pressedMousePos.recalculate(event->globalPos(), frameRect);

        m_ptDragPos = event->globalPos() - frameRect.topLeft();

        if (m_pressedMousePos.m_bOnEdges) {
            if (m_pWidget->isMaximized()) {
                //窗口在最大化状态时，点击边界不做任何处理
                return (false);
            }
            if (d->m_bRubberBandOnResize) {
                m_pRubberBand->setGeometry(frameRect);
                m_pRubberBand->show();
                return (true);
            }
        } else if (d->m_bRubberBandOnMove && m_bLeftButtonTitlePressed) {
            if (m_pWidget->isMaximized()) {
                //窗口在最大化状态时，点击标题栏不做任何处理
                return (false);
            }
            m_pRubberBand->setGeometry(frameRect);
            m_pRubberBand->show();
            return (true);
        }
    }
    return (false);
}


bool WidgetData::handleMouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_bLeftButtonPressed = false;
        m_bLeftButtonTitlePressed = false;
        m_pressedMousePos.reset();
        if (m_pRubberBand && m_pRubberBand->isVisible()) {
            m_pRubberBand->hide();
            m_pWidget->setGeometry(m_pRubberBand->geometry());
            return (true);
        }
    }
    return (false);
}

bool WidgetData::handleMouseMoveEvent(QMouseEvent *event) {
    if (m_bLeftButtonPressed) {
        if (d->m_bWidgetResizable && m_pressedMousePos.m_bOnEdges) {
            if (m_pWidget->isMaximized()) {
                //窗口在最大化状态时，点击边界不做任何处理
                return (false);
            }
            resizeWidget(event->globalPos());
            return (true);
        } else if (d->m_bWidgetMovable && m_bLeftButtonTitlePressed) {
            if (m_pWidget->isMaximized()) {
                //先求出窗口到鼠标的相对位置
                QRect normalGeometry = m_pWidget->normalGeometry();
                m_pWidget->showNormal();
                QPoint p = event->globalPos();
                p.ry() -= 10;
                p.rx() -= (normalGeometry.width() / 2);
                m_pWidget->move(p);
                //这时要重置m_ptDragPos
                m_ptDragPos = QPoint(normalGeometry.width() / 2, 10);
                return (true);
            }
            moveWidget(event->globalPos());
            return (true);
        }
        return (false);
    } else if (d->m_bWidgetResizable) {
        updateCursorShape(event->globalPos());
    }
    return (false);
}


bool WidgetData::handleLeaveEvent(QEvent *event) {
    Q_UNUSED(event)
    if (!m_bLeftButtonPressed) {
        m_pWidget->unsetCursor();
        return (true);
    }
    return (false);
}


bool WidgetData::handleHoverMoveEvent(QHoverEvent *event) {
    if (d->m_bWidgetResizable) {
        updateCursorShape(m_pWidget->mapToGlobal(event->pos()));
    }
    return (false);
}


bool WidgetData::handleDoubleClickedMouseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_pWidget) {
            bool titlePressed = event->pos().y() < m_moveMousePos.m_nTitleHeight;
            if (titlePressed) {
                if (m_pWidget->isMaximized()) {
                    m_pWidget->showNormal();
                } else {
                    m_pWidget->showMaximized();
                }
                return (true);
            }
        }
    }
    return (false);
}

FramelessHelper::FramelessHelper(QObject *parent)
        : QObject(parent),
          m_d(new FramelessHelperPrivate()) {
    m_d->m_bWidgetMovable = true;
    m_d->m_bWidgetResizable = true;
    m_d->m_bRubberBandOnResize = false;
    m_d->m_bRubberBandOnMove = false;
    if (parent) {
        QWidget *w = qobject_cast<QWidget *>(parent);
        if (w) {
            w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
            setWidgetMovable(true);         //设置窗体可移动
            setWidgetResizable(true);       //设置窗体可缩放
            setRubberBandOnMove(false);     //设置橡皮筋效果-可移动
            setRubberBandOnResize(true);    //设置橡皮筋效果-可缩放
            activateOn(w);                  //激活当前窗体
        }
    }
}


FramelessHelper::~FramelessHelper() {
    QList<QWidget *> keys = m_d->m_widgetDataHash.keys();
    int size = keys.size();

    for (int i = 0; i < size; ++i) {
        delete m_d->m_widgetDataHash.take(keys[i]);
    }

    delete m_d;
}


bool FramelessHelper::eventFilter(QObject *obj, QEvent *event) {
    switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Leave: {
            WidgetData *data = m_d->m_widgetDataHash.value(static_cast<QWidget *>(obj));
            if (data) {
                return (data->handleWidgetEvent(event));
            }
            break;
        }

        m_default:
            break;
    }
    return (QObject::eventFilter(obj, event));
}


void FramelessHelper::activateOn(QWidget *topLevelWidget) {
    if (!m_d->m_widgetDataHash.contains(topLevelWidget)) {
        WidgetData *data = new WidgetData(m_d, topLevelWidget);
        m_d->m_widgetDataHash.insert(topLevelWidget, data);

        topLevelWidget->installEventFilter(this);
    }
}


void FramelessHelper::removeFrom(QWidget *topLevelWidget) {
    WidgetData *data = m_d->m_widgetDataHash.take(topLevelWidget);

    if (data) {
        topLevelWidget->removeEventFilter(this);
        delete data;
    }
}


void FramelessHelper::setRubberBandOnMove(bool movable) {
    m_d->m_bRubberBandOnMove = movable;
    QList<WidgetData *> list = m_d->m_widgetDataHash.values();

            foreach(WidgetData *data, list) {
            data->updateRubberBandStatus();
        }
}


void FramelessHelper::setWidgetMovable(bool movable) {
    m_d->m_bWidgetMovable = movable;
}


void FramelessHelper::setWidgetResizable(bool resizable) {
    m_d->m_bWidgetResizable = resizable;
}


void FramelessHelper::setRubberBandOnResize(bool resizable) {
    m_d->m_bRubberBandOnResize = resizable;
    QList<WidgetData *> list = m_d->m_widgetDataHash.values();

            foreach(WidgetData *data, list) {
            data->updateRubberBandStatus();
        }
}


void FramelessHelper::setBorderWidth(uint width) {
    if (width > 0) {
        CursorPosCalculator::m_nBorderWidth = width;
    }
}


void FramelessHelper::setTitleHeight(uint height) {
    if (height > 0) {
        CursorPosCalculator::m_nTitleHeight = height;
    }
}


bool FramelessHelper::widgetMovable() {
    return (m_d->m_bWidgetMovable);
}


bool FramelessHelper::widgetResizable() {
    return (m_d->m_bWidgetResizable);
}


bool FramelessHelper::rubberBandOnMove() {
    return (m_d->m_bRubberBandOnMove);
}


bool FramelessHelper::rubberBandOnResisze() {
    return (m_d->m_bRubberBandOnResize);
}


uint FramelessHelper::borderWidth() {
    return (CursorPosCalculator::m_nBorderWidth);
}


uint FramelessHelper::titleHeight() {
    return (CursorPosCalculator::m_nTitleHeight);
}