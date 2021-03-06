/*
********************************************************************************
*                                嵌入式微系统
*                                   msOS
*
*                            硬件平台:msPLC DEMO
*                          主芯片:STM32F103R8T6/RBT6
*                           深圳市雨滴科技有限公司
*
*                                作者:王绍伟
*                                网名:凤舞天
*                                标识:Wangsw
*
*                                QQ:26033613
*                               QQ群:291235815
*                        论坛:http://bbs.huayusoft.com
*                        淘宝店:http://52edk.taobao.com
*                博客:http://forum.eet-cn.com/BLOG_wangsw317_1268.HTM
********************************************************************************
*文件名     : gui.c
*作用       : 图形库文件
*原理       : 面向对象设计，把一个Menu分为多个Form,一个Form包含多个控件，如Chart、
*           : Label、TextBox等，建立一个最低优先级任务MenuTask去解析执行这些控件信息
********************************************************************************
*版本     作者            日期            说明
*V0.1    Wangsw        2013/07/21       初始版本
********************************************************************************
*/

#include "system.h"

#define MessageSum 8

static char VirtualLcd[4][16];
static char GuiLcd[4][16];
static Message MessageBuffer[MessageSum];
static char Buffer[100];
static Form * FocusFormPointer;

/*******************************************************************************
* 描述	    : 标签解析代码
* 输入参数  : labelPointer 标签指针
* 说明      : sprintf变量支持int和double类型,其它数据类型，尽可能转化为这两者
*******************************************************************************/
static void LabelToGuiLcd(Label * labelPointer)
{
    byte len, x, y;
    int s32;
    uint u32;
    float f32;
    string string;
    
    char format[5];


    x = labelPointer->X;
    y = labelPointer->Y;
        
    switch(labelPointer->Type)
    {
        case GuiDataTypeByteDec:
            u32 = *((byte *)(labelPointer->DataPointer));
            s32 = ((float)u32 + labelPointer->Offset) * labelPointer->Coefficient;
            
            len = sprintf(Buffer, "%d", s32);
            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;
            
        case GuiDataTypeShortDec:
            s32 = *((short *)(labelPointer->DataPointer));
            s32 = ((float)s32 + labelPointer->Offset) * labelPointer->Coefficient;
            
            len = sprintf(Buffer, "%d", s32);
            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;
            
        case GuiDataTypeUshortDec:
            u32 = *((ushort *)(labelPointer->DataPointer));
            s32 = ((float)u32 + labelPointer->Offset) * labelPointer->Coefficient;

            len = sprintf(Buffer, "%d", s32);
            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;
            
        case GuiDataTypeIntDec:                
            s32 = *((int *)(labelPointer->DataPointer));
            s32 = ((double)s32 + (double)labelPointer->Offset) * (double)labelPointer->Coefficient;

            len = sprintf(Buffer, "%d", s32);
            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;
            
        case GuiDataTypeUintDec:
            u32 = *((uint *)(labelPointer->DataPointer));
            u32 = ((double)u32 + (double)labelPointer->Offset) * (double)labelPointer->Coefficient;

            len = sprintf(Buffer, "%u", u32);
            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;

        case GuiDataTypeFloatDec:
            f32 = *((float *)(labelPointer->DataPointer));
            f32 = (f32 + labelPointer->Offset) * labelPointer->Coefficient;            

            memcpy(format, "%.nf", 5); format[2] = Ascii(labelPointer->Digits);
            len = sprintf(Buffer, format, (double)f32);

            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;
            
        case GuiDataTypeUshortHex:
            u32 = *((ushort *)(labelPointer->DataPointer));
            u32 = ((float)u32 + labelPointer->Offset) * labelPointer->Coefficient;
            len = labelPointer->Digits;
            
            memcpy(format, "%0nX", 5); format[2] = Ascii(len);
            sprintf(Buffer, format, u32); 

            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;   

        case GuiDataTypeUintHex:
            u32 = *((uint *)(labelPointer->DataPointer));
            u32 = ((double)u32 + (double)labelPointer->Offset) * (double)labelPointer->Coefficient;
            len = labelPointer->Digits;

            memcpy(format, "%0nX", 5); format[2] = Ascii(len);
            sprintf(Buffer, format, u32); 

            while(len--) {GuiLcd[y][x] = Buffer[len]; x--;}
            break;

        case GuiDataTypeString:
            string = *(char **)labelPointer->DataPointer;
            len = strlen(string);
            if(labelPointer->Align == GuiDataAlignRight) x = x + 1 - len;
            while(len--) {GuiLcd[y][x] = *string++; x++;}
   
            break;

        case GuiDataTypeSnString:
            u32 = *((byte *)(labelPointer->DataPointer));
            string = labelPointer->StringBlockPointer[u32];
            len = strlen(string);
            if(labelPointer->Align == GuiDataAlignRight) x = x + 1 - len;
            while(len--) {GuiLcd[y][x] = *string++; x++;}
            break;
    }
}

/*******************************************************************************
* 描述	    : 背景文字扫描刷新
* 输入参数  : formPointer 当前窗体指针
*******************************************************************************/
static void ParseBackText(void)
{
    const byte * backTextPointer;
    backTextPointer = FocusFormPointer->BackTextPointer;
    
    if(backTextPointer == null)
        memset(GuiLcd, ' ', 64);  
    else
        memcpy(GuiLcd, backTextPointer, 64);
}


/*******************************************************************************
* 描述	    : Chart图形表控件扫描刷新
* 输入参数  : formPointer 当前窗体指针
*******************************************************************************/
static void ParseChart(void)
{
    byte i, j;
    char character;
    byte column;
    Chart * chartPointer;

    chartPointer = FocusFormPointer->ChartPointer;
    if(chartPointer == null) return;
        
    character = chartPointer->Character;
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 16; j++)
        {
            column = chartPointer->Column[j];
            Assert(column < 5);
            
            if(column >= (4 - i))
                GuiLcd[i][j] = character;
            else
                GuiLcd[i][j] = ' ';
        }
    }
}

/*******************************************************************************
* 描述	    : Label控件扫描刷新，Label一般用于文本显示，但不能按键输入操作
* 输入参数  : formPointer 当前窗体指针
*******************************************************************************/
static void ParseLabel(void)
{
    Label * labelPointer;
    
    labelPointer = FocusFormPointer->LabelPointer;

    while(labelPointer != null)
    {
        LabelToGuiLcd(labelPointer);
        labelPointer = labelPointer->NextLabelPointer;
    }
}

/*******************************************************************************
* 描述	    : TextBox控件扫描刷新，TextBox不仅用于文本显示，还能按键输入修改参数
* 输入参数  : formPointer 当前窗体指针
*******************************************************************************/
static void ParseTextBox(void)
{
    static byte Counter;
    
    TextBox * textBoxPointer;

    textBoxPointer = FocusFormPointer->TextBoxPointer;

    while(textBoxPointer != null)
    {
        if (FocusFormPointer->FocusTextBoxPointer == textBoxPointer)
        {
            if(++Counter == 8) Counter = 0;
            if (Counter < 4)
            {
                textBoxPointer = textBoxPointer->NextTextBoxPointer;
                continue;
            }
        }
        LabelToGuiLcd((Label *)textBoxPointer);
        
        textBoxPointer = textBoxPointer->NextTextBoxPointer;
    }
}
static void ParseMessage(void)
{
    int i;
    Message * pointer;
    pointer = &MessageBuffer[0];
    for (i = 0; i < MessageSum; i++)
    {
        if (pointer->Valid)
        {
            memcpy(&GuiLcd[pointer->Y][pointer->X], pointer->Buffer, pointer->Len);
        }
        pointer++;
    }
}

/*******************************************************************************
* 描述	    : LCD屏数据更新，发现显示数据有变化，按行更新LCD屏内容
*******************************************************************************/
static void Update(void)
{
#ifdef LCD12864
    byte i, j;
    bool update;
    for (i = 0; i < 4; i++)
	{
        update = false;
        
        for (j = 0; j < 16; j++)
        {
            if(VirtualLcd[i][j] != GuiLcd[i][j])
            {
                VirtualLcd[i][j] = GuiLcd[i][j];
                update = true;
            }
        }
        if (update)
            System.Device.Lcd.DisplayString(i, VirtualLcd[i]);
    }
#endif
#ifdef msMenu
    byte i;
    bool update;
    
    update = false;
    for (i = 0; i < 64; i++)
	{
        if(*((char *)VirtualLcd + i) != *((char *)GuiLcd + i))
        {
            *((char *)VirtualLcd + i) = *((char *)GuiLcd + i);
            update = true;
        }
    }
    if (update == true)
    {
        System.Device.Usart1.WriteByte(0x02);
        for (i = 0; i < 64; i+=16)
        {
            System.Device.Usart1.Write((byte *)VirtualLcd + i, 16);
            System.Device.Usart1.WriteByte(0x0D);
            System.Device.Usart1.WriteByte(0x0A);
        }
        System.Device.Usart1.WriteByte(0x03);
    }
#endif
}

/*******************************************************************************
* 描述	    : 解析页面
*******************************************************************************/
static void Parse(Form * formPointer)
{  
    FocusFormPointer = formPointer;
    
    ParseBackText();            //背景文字控件扫描解析

    ParseChart();               //表控件扫描解析

    ParseLabel();               //标签控件扫描解析

    ParseTextBox();             //文本控件扫描解析
    ParseMessage();

    Update();                   //更新LCD显示内容
}

/*******************************************************************************
* 描述	    : 初始化页面
* 输入参数  : formPointer :窗体Form指针
*******************************************************************************/
static void InitForm(Form *formPointer)
{
    formPointer->BackTextPointer = null;
    formPointer->ChartPointer = null;
    formPointer->LabelPointer = null;
    formPointer->TextBoxPointer = null;
    formPointer->FocusTextBoxPointer = null;
}

/*******************************************************************************
* 描述	    : 窗体Form加载Label，加载时清空Label内容，加载后，需要对Label初始化内容
* 输入参数  : formPointer :窗体Form指针
*           : labelPointer: 控件Label指针
*******************************************************************************/
static void AddLabel(Form * formPointer, Label *labelPointer)
{
    labelPointer->DataPointer = null;
    
    labelPointer->Digits= 0;
    labelPointer->Offset = 0;
    labelPointer->Coefficient = 1;
    
    labelPointer->StringBlockPointer = null;
    labelPointer->Align = GuiDataAlignRight;
    labelPointer->NextLabelPointer = formPointer->LabelPointer;
    formPointer->LabelPointer = labelPointer;
}


/*******************************************************************************
* 描述	    : 窗体Form加载TextBox，加载时清空TextBox内容，加载后，需要对TextBox初始化内容
* 输入参数  : formPointer :窗体Form指针
*           : textBoxPointer: 控件TextBox指针
*******************************************************************************/
static void AddTextBox(Form * formPointer, TextBox *textBoxPointer)
{    
    TextBox * pointer;
    
    textBoxPointer->DataPointer = null;
    textBoxPointer->DataMax = 0;
    textBoxPointer->DataMin = 0;
    textBoxPointer->DataStep = 0;
    textBoxPointer->DataBigStep = 0;
    
    textBoxPointer->Digits= 0;
    textBoxPointer->Offset = 0;
    textBoxPointer->Coefficient = 1;
    
    textBoxPointer->StringBlockPointer = null;
    textBoxPointer->Align = GuiDataAlignRight;
    textBoxPointer->NextTextBoxPointer = null;
    textBoxPointer->FunctionPointer = null;
    
    if (formPointer->TextBoxPointer == null)
    {
         formPointer->TextBoxPointer = textBoxPointer;
         return;
    }
    
    pointer = formPointer->TextBoxPointer;
        
    while (pointer->NextTextBoxPointer != null)
        pointer = pointer->NextTextBoxPointer;

    pointer->NextTextBoxPointer = textBoxPointer;
}

static void AddMessage(int id, int x, int y, char *fmt, ...)
{
    int len;
    va_list list;

    va_start(list, fmt);
    len = vsprintf(MessageBuffer[id].Buffer, fmt, list);
    va_end(list);

    if (len == 0) return;
    if (id >= MessageSum) return;

    MessageBuffer[id].X = x;
    MessageBuffer[id].Y = y;
    MessageBuffer[id].Len = len;
    MessageBuffer[id].Valid = true;
}

static void DeleteMessage(int id)
{
    MessageBuffer[id].Valid = false;
}

/*******************************************************************************
* 描述	    : 文本控件焦点切换处理程序，切换文本控件焦点
*******************************************************************************/
static void SwitchTextBoxFocus(void)
{
    if (FocusFormPointer->FocusTextBoxPointer == null)
    {
        FocusFormPointer->FocusTextBoxPointer = FocusFormPointer->TextBoxPointer;
        return;
    }
    
    FocusFormPointer->FocusTextBoxPointer = FocusFormPointer->FocusTextBoxPointer->NextTextBoxPointer;
}

/*******************************************************************************
* 描述	    : 文本控件数值修改处理修改程序
* 输入参数  : key: AddKey、AddLongKey、SubKey、SubLongKey
*           :      AssistUpKey、AssistUpLongKey、AssistDownKey、AssistDownLongKey
*******************************************************************************/
static void ModifyTextBoxData(KeyEnum key)
{
    TextBox * textBoxPointer;
    void * dataPointer;
    GuiDataType dataType;
    
    int s32;
    int s32Max;
    int s32Min;
    int s32Step;
    int s32BigStep;
    
    uint u32;
    uint u32Max;
    uint u32Min;
    uint u32Step;
    uint u32BigStep;

    float f32;
    float f32Max;
    float f32Min;
    float f32Step;
    float f32BigStep;
    

    textBoxPointer = FocusFormPointer->FocusTextBoxPointer;
    if (textBoxPointer == null) return;
    
    dataPointer = textBoxPointer->DataPointer;
    dataType = textBoxPointer->Type;
    
    switch(dataType)
    {
//带符号整数
        case GuiDataTypeShortDec:
            s32 = * (short *)(textBoxPointer->DataPointer);
            s32Max = (short)(textBoxPointer->DataMax);
            s32Min = (short)(textBoxPointer->DataMin);
            s32Step = (short)(textBoxPointer->DataStep);        
            s32BigStep = (short)(textBoxPointer->DataBigStep);
            goto ProcInt;
        case GuiDataTypeIntDec:
            s32 = * (int *)(textBoxPointer->DataPointer);
            s32Max = (int)(textBoxPointer->DataMax);
            s32Min = (int)(textBoxPointer->DataMin);
            s32Step = (int)(textBoxPointer->DataStep);        
            s32BigStep = (int)(textBoxPointer->DataBigStep);
            
ProcInt:    switch(key)
            {
                case KeyAdd: s32 += s32Step; break;
                case KeyLongAdd: s32 += s32BigStep; break;
                case KeySub: s32 -= s32Step; break;
                case KeyLongSub: s32 -= s32BigStep; break;
            }
            if (s32 > s32Max) s32 = s32Max; 
            else if (s32 < s32Min) s32 = s32Min;

            if (dataType == GuiDataTypeIntDec) *(int *)dataPointer = s32; 
            else *(short *)dataPointer = s32;
            break;
//无符号整数
        case GuiDataTypeSnString:
        case GuiDataTypeByteDec:
            u32 = * (byte *)(textBoxPointer->DataPointer);
            u32Max = (byte)(textBoxPointer->DataMax);
            u32Min = (byte)(textBoxPointer->DataMin);
            u32Step = (byte)(textBoxPointer->DataStep);        
            u32BigStep = (byte)(textBoxPointer->DataBigStep);
            goto ProcUint;
        case GuiDataTypeUshortDec:
        case GuiDataTypeUshortHex:
            u32 = * (ushort *)(textBoxPointer->DataPointer);
            u32Max = (ushort)(textBoxPointer->DataMax);
            u32Min = (ushort)(textBoxPointer->DataMin);
            u32Step = (ushort)(textBoxPointer->DataStep);        
            u32BigStep = (ushort)(textBoxPointer->DataBigStep);
            goto ProcUint;
        case GuiDataTypeUintDec:
        case GuiDataTypeUintHex:
            u32 = * (uint *)(textBoxPointer->DataPointer);            
            u32Max = (uint)(textBoxPointer->DataMax);
            u32Min = (uint)(textBoxPointer->DataMin);
            u32Step = (uint)(textBoxPointer->DataStep);        
            u32BigStep = (uint)(textBoxPointer->DataBigStep);
            
ProcUint:   switch(key)
            {
                case KeyAdd: u32 += u32Step; break;
                case KeyLongAdd: u32 += u32BigStep; break;
                case KeySub: if (u32 < u32Step) u32 = 0; else u32 -= u32Step; break;
                case KeyLongSub: if (u32 < u32BigStep) u32 = 0; else u32 -= u32BigStep; break;
            }  
            if (u32 > u32Max) u32 = u32Max;
            else if (u32 < u32Min) u32 = u32Min;

            switch(dataType)
            {   
                case GuiDataTypeSnString:
                case GuiDataTypeByteDec:
                    *(byte *)dataPointer = u32;
                    break;
                case GuiDataTypeUshortDec:
                case GuiDataTypeUshortHex:
                    *(ushort *)dataPointer = u32;
                    break;
                case GuiDataTypeUintDec:
                case GuiDataTypeUintHex:
                    *(uint *)dataPointer = u32;
                    break;
            }
            break;
//单精度浮点数
        case GuiDataTypeFloatDec:
            f32 = * (float *)(textBoxPointer->DataPointer);
            f32Max = Float(textBoxPointer->DataMax);
            f32Min = Float(textBoxPointer->DataMin);
            f32Step = Float(textBoxPointer->DataStep);       
            f32BigStep = Float(textBoxPointer->DataBigStep);
            
            switch(key)
            {
                case KeyAdd: f32 += f32Step; break;
                case KeyLongAdd: f32 += f32BigStep; break;
                case KeySub: f32 -= f32Step; break;
                case KeyLongSub: f32 -= f32BigStep; break;
            }  
            if (f32 > f32Max) f32 = f32Max;
            else if (f32 < f32Min) f32 = f32Min;

            *(float *)dataPointer = f32;
            break;
        default:
            break;
    }
    if (textBoxPointer->FunctionPointer != null)
        textBoxPointer->FunctionPointer();
}

void InitGui(void)
{
    byte i;
    for (i = 0; i < 64; i++)
    {
        ((byte *)GuiLcd)[i] = ' ';
        ((byte *)VirtualLcd)[i] = ' ';
    }
    
    System.Gui.Parse = Parse;
    
    System.Gui.Form.AddLabel = AddLabel;

    System.Gui.Form.AddTextBox = AddTextBox;

    System.Gui.Form.Init = InitForm;    

    System.Gui.Form.SwitchTextBoxFocus = SwitchTextBoxFocus;

    System.Gui.Form.ModifyTextBoxData = ModifyTextBoxData;

    System.Gui.Form.AddMessage = AddMessage;
    
    System.Gui.Form.DeleteMessage = DeleteMessage;
}



