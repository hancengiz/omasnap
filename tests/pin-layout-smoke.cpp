/** @fileoverview Tests stacked pin slots and drag clamping. */
#include "pin-layout-smoke.hpp"

#include "pin-layout.hpp"

bool runPinLayoutSmoke(QString &error) {
  // Use compositor coordinates, including a monitor with a nonzero origin.
  const QPoint firstDrag = pinPositionFromGlobalPointer(
      QPoint(446, 306), QPoint(100, 50), QPoint(50, 40));
  const QPoint secondDrag = pinPositionFromGlobalPointer(
      QPoint(456, 316), QPoint(100, 50), QPoint(50, 40));
  if (firstDrag != QPoint(296, 216) || secondDrag != QPoint(306, 226)) {
    error = QStringLiteral("Pin drag did not follow the compositor pointer");
    return false;
  }
  // Logical coordinates must not be rescaled, even on an offset, scaled
  // output. Layer-shell margins and hyprctl cursorpos both use logical pixels.
  if (pinPositionFromGlobalPointer(QPoint(694, 646), QPoint(480, 0),
                                   QPoint(100, 100)) != QPoint(114, 546) ||
      pinPositionFromGlobalPointer(QPoint(-1200, 400), QPoint(-1536, 0),
                                   QPoint(36, 100)) != QPoint(300, 300)) {
    error = QStringLiteral("Pin drag mishandled an offset monitor");
    return false;
  }

  const QSize screen(400, 300);
  const QSize pin(100, 80);
  const QPoint first = pinSlotPosition(screen, pin, pin, 0, 10, 14);
  const QPoint second = pinSlotPosition(screen, pin, pin, 1, 10, 14);
  const QPoint third = pinSlotPosition(screen, pin, pin, 2, 10, 14);
  const QPoint wrapped = pinSlotPosition(screen, pin, pin, 3, 10, 14);
  if (first != QPoint(14, 206) || second != QPoint(14, 116) ||
      third != QPoint(14, 26) || wrapped != QPoint(124, 206)) {
    error = QStringLiteral("Pinned slots did not stack and wrap correctly");
    return false;
  }

  const QSize cell(120, 90);
  const QRect large(pinSlotPosition(screen, QSize(120, 90), cell, 0, 10, 14),
                    QSize(120, 90));
  const QRect small(pinSlotPosition(screen, QSize(60, 40), cell, 1, 10, 14),
                    QSize(60, 40));
  if (large.intersects(small)) {
    error = QStringLiteral("Different pin sizes overlapped their layout slots");
    return false;
  }

  const QRect bounds(QPoint(10, 20), QSize(400, 300));
  if (clampPinGeometry(QRect(380, 290, 100, 80), bounds) !=
      QRect(310, 240, 100, 80)) {
    error = QStringLiteral("Pinned geometry was not clamped to the screen");
    return false;
  }
  if (clampPinGeometry(QRect(-20, 0, 100, 80), bounds) !=
      QRect(10, 20, 100, 80)) {
    error = QStringLiteral("Pinned geometry did not clamp at top-left");
    return false;
  }
  return true;
}
