/*
*********************************************************************************************************
*	                                  
*	ģ������ : GUI����������
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : Shows how to use a numpad as input device on a touch screen
*	�޸ļ�¼ :
*		�汾��    ����          ����                 ˵��
*		v1.0    2013-04-22    Eric2013      ST�̼���汾 V1.0.2�汾��
*
*	Copyright (C), 2013-2014
*   QQ����Ⱥ��216681322
*   BLOG: http://blog.sina.com.cn/u/2565749395 
*********************************************************************************************************
*/

#include "MainTask.h"


int i;
char acText[] = "www.armfly.com  www.armfly.taobao.com Eric2013";
GUI_RECT Rect = {10, 10, 59, 59};
GUI_WRAPMODE aWm[] = {GUI_WRAPMODE_NONE,
                      GUI_WRAPMODE_CHAR,
                      GUI_WRAPMODE_WORD};
/*
*********************************************************************************************************
*	�� �� ��: MainTask
*	����˵��: GUI������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MainTask(void) 
{
    GUI_Init();
    GUI_SetTextMode(GUI_TM_TRANS);
    for (i = 0; i < 3; i++) 
    {
        GUI_SetColor(GUI_BLUE);
        GUI_FillRectEx(&Rect);
        GUI_SetColor(GUI_WHITE);
        GUI_DispStringInRectWrap(acText, &Rect, GUI_TA_LEFT, aWm[i]);
        Rect.x0 += 60;
        Rect.x1 += 60;
    }
    while (1)
    {
        GUI_Delay(10);
    }
}

