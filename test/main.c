#include <nvui/window.h>
#include <nvui/painter.h>
#include <nvui/widgets.h>
#include <stdio.h>
#include <string.h>

int FixedReportedSizeElement(Element *element, Message message, int di, void *dp) {
	(void) di;

	if (message == MSG_GET_WIDTH) {
		return 25;
	} else if (message == MSG_GET_HEIGHT) {
		return 50;
	} else if (message == MSG_PAINT) {
		Painter *painter = dp;
		PainterSetColor(painter, ColorFromInt((uint32_t)(uintptr_t)ElementGetContext(element)));
		PainterFillRect(painter, ElementGetBounds(element));
	}

	return 0;
}

int AspectRatioElement(Element *element, Message message, int di, void *dp) {
	if (message == MSG_GET_WIDTH) {
		return di * 2;
	} else if (message == MSG_GET_HEIGHT) {
		return di / 2;
	} else if (message == MSG_PAINT) {
		Painter *painter = dp;
		PainterSetColor(painter, ColorFromInt((uint32_t)(uintptr_t)ElementGetContext(element)));
		PainterFillRect(painter, ElementGetBounds(element));
	}

	return 0;
}

int FixedAreaElement(Element *element, Message message, int di, void *dp) {
	if (message == MSG_GET_WIDTH) {
		return di ? 50000 / di : 0;
	} else if (message == MSG_GET_HEIGHT) {
		return di ? 50000 / di : 0;
	} else if (message == MSG_PAINT) {
		Painter *painter = dp;
		PainterSetColor(painter, ColorFromInt((uint32_t)(uintptr_t)ElementGetContext(element)));
		PainterFillRect(painter, ElementGetBounds(element));
	}

	return 0;
}

int main() {
	Initialize();

	Window *window1 = WindowCreate("Window 1", 1000, 800);

	FlowPanel *column1 = FlowPanelCreate(WindowGetRootElement(window1), PANEL_GRAY);
	FlowPanelSetGap(column1, 10);
	FlowPanelSetBorder(column1, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column1, 0, "Label 1", -1);
	LabelCreate((Element*)column1, 0, "Longer label 2", -1);

	FlowPanel *column2 = FlowPanelCreate((Element*)column1, PANEL_WHITE);
	FlowPanelSetGap(column2, 10);
	FlowPanelSetBorder(column2, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column2, ELEMENT_H_FILL, "Label 3", -1);
	LabelCreate((Element*)column2, ELEMENT_H_FILL, "Much Longer label 4", -1);

	FlowPanel *column3 = FlowPanelCreate((Element*)column1, PANEL_WHITE | ELEMENT_H_FILL);
	FlowPanelSetGap(column2, 10);
	FlowPanelSetBorder(column2, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column3, 0, "Label 4", -1);
	LabelCreate((Element*)column3, 0, "Longer label 5", -1);

	FlowPanel *column4 = FlowPanelCreate((Element*)column1, PANEL_WHITE | ELEMENT_H_FILL);
	FlowPanelSetGap(column4, 10);
	FlowPanelSetBorder(column4, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column4, ELEMENT_H_FILL, "Label 6", -1);
	LabelCreate((Element*)column4, ELEMENT_H_FILL, "Longer label 7", -1);

	ButtonCreate((Element*)column1, ELEMENT_V_FILL, "Vertical fill button 1", -1);
	ButtonCreate((Element*)column1, ELEMENT_V_FILL, "Vertical fill button 2", -1);
	ButtonCreate((Element*)column1, ELEMENT_V_FILL | ELEMENT_H_FILL, "Vertical and horizontal fill button 3", -1);
	ButtonCreate((Element*)column1, ELEMENT_V_FILL, "Vertical fill button 4", -1);

	FlowPanel *row1 = FlowPanelCreate((Element*)column1, PANEL_WHITE | PANEL_HORIZONTAL);
	FlowPanelSetGap(row1, 10);
	FlowPanelSetBorder(row1, (Rectangle){ 10, 10, 10, 10 });

	ButtonCreate((Element*)row1, 0, "Button 1 in row", -1);
	ButtonCreate((Element*)row1, ELEMENT_H_FILL, "Button 2 in row", -1);
	ButtonCreate((Element*)row1, 0, "Button 3 in row", -1);

	FlowPanel *row2 = FlowPanelCreate((Element*)column1, PANEL_WHITE | PANEL_HORIZONTAL | ELEMENT_H_FILL);
	FlowPanelSetGap(row2, 10);
	FlowPanelSetBorder(row2, (Rectangle){ 10, 10, 10, 10 });

	ButtonCreate((Element*)row2, 0, "Button 4 in row", -1);
	ButtonCreate((Element*)row2, ELEMENT_H_FILL, "Button 5 in row", -1);
	ButtonCreate((Element*)row2, 0, "Button 6 in row", -1);

	Window *window2 = WindowCreate("Window 2", 500, 500);

	FlowPanel *column5 = FlowPanelCreate(WindowGetRootElement(window2), PANEL_GRAY);
	FlowPanelSetGap(column5, 10);
	FlowPanelSetBorder(column5, (Rectangle){ 10, 10, 10, 10 });

	Element *el = ElementCreate(ElementSize, (Element*)column5, 0, FixedReportedSizeElement);
	ElementSetContext(el, (void *) (uintptr_t) 0x111111);
	el = ElementCreate(ElementSize, (Element*)column5, ELEMENT_H_FILL, FixedReportedSizeElement);
	ElementSetContext(el, (void *) (uintptr_t) 0xFF1111);
	el = ElementCreate(ElementSize, (Element*)column5, ELEMENT_V_FILL, FixedReportedSizeElement);
	ElementSetContext(el, (void *) (uintptr_t) 0x11FF11);
	el = ElementCreate(ElementSize, (Element*)column5, ELEMENT_H_FILL | ELEMENT_V_FILL, FixedReportedSizeElement);
	ElementSetContext(el, (void *) (uintptr_t) 0x1111FF);

	Window *window3 = WindowCreate("Window 3", 500, 500);
	FlowPanel *column6 = FlowPanelCreate(WindowGetRootElement(window3), PANEL_GRAY);
	FlowPanelSetGap(column6, 10);
	FlowPanelSetBorder(column6, (Rectangle){ 10, 10, 10, 10 });

	el = ElementCreate(ElementSize, (Element*)column6, ELEMENT_H_FILL, AspectRatioElement);
	ElementSetContext(el, (void *) (uintptr_t) 0x111111);
	el = ElementCreate(ElementSize, (Element*)column6, ELEMENT_V_FILL, AspectRatioElement);
	ElementSetContext(el, (void *) (uintptr_t) 0xFF1111);
	el = ElementCreate(ElementSize, (Element*)column6, ELEMENT_H_FILL, FixedAreaElement);
	ElementSetContext(el, (void *) (uintptr_t) 0x11FF11);
	el = ElementCreate(ElementSize, (Element*)column6, ELEMENT_V_FILL, FixedAreaElement);
	ElementSetContext(el, (void *) (uintptr_t) 0x1111FF);

	return MessageLoop();
}
