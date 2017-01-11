#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>


#define X_BLOCKS 30
#define Y_BLOCKS 20
#define BG 0
#define PLAYER 1
#define CUBE 2
#define GROUND 3
#define END 4
#define BOMB 5

int pole[X_BLOCKS][Y_BLOCKS];
int read[X_BLOCKS*Y_BLOCKS];
int direction[4][2] = {0, -1, 1, 0, 0, 1, -1, 0};
int direct;
int actual_level;
BOOL correct_move;
BOOL refresh;
BOOL moved_cube;
BOOL active_keys[4];
BOOL stop;
int dead = 0;


FILE *fp;
POINT player, move_player, move_cube;


void load(int level);
void find_player(void);
void move(int speed);
void fall(void);
void push(void);
void fall_cube(void);
void search_bomb(void);
void around_bomb(int i, int j, int znak);
void search_fall(void);
void around_fall(int i, int j);

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("Cohesion") ;
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS     wndclass ;

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;
     
     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("Program vyžaduje Windows NT!"), 
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     
     hwnd = CreateWindow (szAppName, TEXT ("Cohesion"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL) ;
     
     ShowWindow (hwnd, SW_MAXIMIZE) ;
     UpdateWindow (hwnd) ;
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     HDC         hdc ;
     int         wnd_x, wnd_y ;
     PAINTSTRUCT ps ;
     POINT       point ;
     RECT        rect ;
     static int  cxBlock, cyBlock ;
     HPEN        hPen;
     HPEN        hPen_player;
     HPEN        hPen_cube;
     HPEN        hPen_ground;
     HPEN        hPen_bomb;
     HBRUSH      hBrush_player;
     HBRUSH      hBrush_cube;
     HBRUSH      hBrush_ground;
     HBRUSH      hBrush_bomb;
     int         x, y;
     
     hPen = CreatePen(PS_SOLID, 1, RGB(0, 40, 0));
     hPen_player = CreatePen(PS_SOLID, 2, RGB(255, 176, 43));
     hPen_cube = CreatePen(PS_SOLID, 2, RGB(0, 0, 180));
     hPen_ground = CreatePen(PS_SOLID, 2, RGB(50, 150, 50));
     hPen_bomb = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
     
     hBrush_player = CreateSolidBrush(RGB(255, 176, 43));
     hBrush_cube = CreateSolidBrush(RGB(0, 0, 255));
     hBrush_ground = CreateSolidBrush(RGB(50, 150, 50));
     hBrush_bomb = CreateSolidBrush(RGB(255, 255, 50));
     
     switch (message)
     {
     case WM_CREATE:
          load(1);
          find_player();
          //hdc = GetDC (hwnd);
          //GetTextMetrics(hdc. &tm);
          return 0;
          
     case WM_SIZE :
          cxBlock = LOWORD (lParam) / X_BLOCKS ;
          cyBlock = HIWORD (lParam) / Y_BLOCKS ;
          return 0 ;
     
     case WM_SETFOCUS :
          ShowCursor (TRUE) ;
          return 0 ;
          
     case WM_KILLFOCUS :
          ShowCursor (FALSE) ;
          return 0 ;
          
     case WM_KEYDOWN :       
          switch (wParam)
          {
          case 49 :
               load(1);
               break ;
               
          case 50 :
               load(2);
               break ;
               
          case 51 :
               load(3);
               break ;
               
          case 52 :
               load(4);
               break ;
               
          case 53 :
               load(5);
               break ;
               
          case 54 :
               load(6);
               break ;
               
          case 55 :
               load(7);
               break ;
               
          case 56 :
               load(8);
               break ;

          case 57 :
               load(9);
               break ;
               
          case 82 :
               load(actual_level);
               break ;
               
          case VK_UP :
               direct=0 ;
               active_keys[0] = 1;
               break ;
               
          case VK_DOWN :
               direct=2 ;
               active_keys[2] = 1;
               move(1);
               break ;
               
          case VK_LEFT :
               direct=3 ;
               active_keys[3] = 1;
               move(1);
               break ;
               
          case VK_RIGHT :
               direct=1 ;
               active_keys[1] = 1;
               move(1);
               break ;

          case VK_SPACE :
               break ;
          }
          search_fall();
          search_bomb();
          if(refresh)
          InvalidateRect (hwnd, NULL, FALSE) ;
          
          if(correct_move){
          rect.left   = player.x * cxBlock ;
          rect.top    = player.y * cyBlock ;
          rect.right  = (player.x + 1) * cxBlock ;
          rect.bottom = (player.y + 1) * cyBlock ;
          InvalidateRect (hwnd, &rect, FALSE) ;
          rect.left   = move_player.x * cxBlock ;
          rect.top    = move_player.y * cyBlock ;
          rect.right  = (move_player.x + 1) * cxBlock ;
          rect.bottom = (move_player.y + 1) * cyBlock ;
          InvalidateRect (hwnd, &rect, FALSE) ;
          
          player.x = move_player.x;
          player.y = move_player.y;
          }
          if(moved_cube){
          rect.left   = move_cube.x * cxBlock ;
          rect.top    = move_cube.y * cyBlock ;
          rect.right  = (move_cube.x + 1) * cxBlock ;
          rect.bottom = (move_cube.y + 1) * cyBlock ;
          InvalidateRect (hwnd, &rect, FALSE) ;
          }
          correct_move = 0;
          moved_cube = 0;
          stop = 0;
          return 0 ;
          
     case WM_KEYUP :
          switch (wParam)
          {
          case VK_UP :
               active_keys[0] = 0;
               break ;
               
          case VK_DOWN :
               active_keys[2] = 0;
               break ;
               
          case VK_LEFT :
               active_keys[3] = 0;
               break ;
               
          case VK_RIGHT :
               active_keys[1] = 0;
               break ;

          case VK_SPACE :
               MessageBox(NULL, TEXT("KUK"), TEXT("Message Box"), 0);
               break ;
          }  
          return 0 ;
          
     case WM_PAINT :
          hdc = BeginPaint (hwnd, &ps) ;
          for (wnd_x = 0 ; wnd_x < X_BLOCKS ; wnd_x++)
          for (wnd_y = 0 ; wnd_y < Y_BLOCKS ; wnd_y++)
          {
               SelectObject(hdc, hPen);
               SelectObject(hdc, GetStockObject(BLACK_BRUSH));
               Rectangle (hdc, wnd_x * cxBlock, wnd_y * cyBlock,
                          (wnd_x + 1) * cxBlock, (wnd_y + 1) * cyBlock) ;
 
               if (pole[wnd_x][wnd_y]==PLAYER)
               {
                    SelectObject(hdc, hPen_player);
                    SelectObject(hdc, hBrush_player);
                    Ellipse (hdc, wnd_x*cxBlock+2, wnd_y*cyBlock+2, (wnd_x+1)*cxBlock-2, (wnd_y+1)*cyBlock-2);   
               }
               else if(pole[wnd_x][wnd_y]==CUBE)
               {
                    SelectObject(hdc, hPen_cube);
                    SelectObject(hdc, hBrush_cube);
                    RoundRect(hdc, wnd_x*cxBlock+2, wnd_y*cyBlock+2, (wnd_x+1)*cxBlock-2, (wnd_y+1)*cyBlock-2, 5, 5);
               }
               else if(pole[wnd_x][wnd_y]==GROUND)
               {
                    SelectObject(hdc, hPen_ground);
                    SelectObject(hdc, hBrush_ground);
                    Rectangle(hdc, wnd_x*cxBlock+2, wnd_y*cyBlock+2, (wnd_x+1)*cxBlock-2, (wnd_y+1)*cyBlock-2);
               }
               else if(pole[wnd_x][wnd_y]==END)
               {
                    SelectObject(hdc, hPen_player);
                    SelectObject(hdc, GetStockObject(BLACK_BRUSH));
                    Rectangle(hdc, wnd_x*cxBlock+2, wnd_y*cyBlock+2, (wnd_x+1)*cxBlock-2, (wnd_y+1)*cyBlock-2);
               }
               else if(pole[wnd_x][wnd_y]==BOMB)
               {
                    SelectObject(hdc, hPen_bomb);
                    SelectObject(hdc, hBrush_bomb);
                    RoundRect(hdc, wnd_x*cxBlock+2, wnd_y*cyBlock+2, (wnd_x+1)*cxBlock-2, (wnd_y+1)*cyBlock-2, 5, 5);
               }
          }

          EndPaint (hwnd, &ps) ;
          return 0 ;
               
     case WM_DESTROY :
          DeleteObject(hPen);
          DeleteObject(hPen_player);
          DeleteObject(hPen_cube);
          DeleteObject(hPen_ground);
          DeleteObject(hPen_bomb);
          DeleteObject(hBrush_player);
          DeleteObject(hBrush_cube);
          DeleteObject(hBrush_ground);
          DeleteObject(hBrush_bomb);
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void load(int level){
     char str[10];
     switch(level){
     case 1 :
          strcpy(str, "level_1");
          actual_level = 1;
          break;
     case 2 :
          strcpy(str, "level_2");
          actual_level = 2;
          break;
     case 3 :
          strcpy(str, "level_3");
          actual_level = 3;
          break;
     case 4 :
          strcpy(str, "level_4");
          actual_level = 4;
          break;
     case 5 :
          strcpy(str, "level_5");
          actual_level = 5;
          break;
     case 6 :
          strcpy(str, "level_6");
          actual_level = 6;
          break;
     case 7 :
          strcpy(str, "level_7");
          actual_level = 7;
          break;
     case 8 :
          strcpy(str, "level_8");
          actual_level = 8;
          break;
     case 9 :
          strcpy(str, "level_9");
          actual_level = 9;
          break;
     }

     if((fp = fopen(str , "r"))==NULL){
          MessageBox(NULL, TEXT("Soubor nelze otevrit"), TEXT("Message Box"), 0);
          exit(1);
     }

     for(int j=0; j < Y_BLOCKS; j++){
        for(int i=0; i < X_BLOCKS; i++){
                 fscanf(fp, "%d", &pole[i][j]);
        }
     }
     fclose(fp);
     refresh = 1;
     find_player();
}

void find_player(void){
     for(int j=0; j < Y_BLOCKS; j++){
        for(int i=0; i < X_BLOCKS; i++){
              if(pole[i][j]==1){
              player.x = i;
              player.y = j;
              }
        }
     }
}

void move(int speed){
     move_player.x = player.x + direction[direct][0];
     move_player.y = player.y + direction[direct][1];

     
     if(move_player.x >= 0 && move_player.x < X_BLOCKS){
     if(move_player.y >= 0 && move_player.y < X_BLOCKS){            
     if(active_keys[0]){
     if(pole[player.x][player.y-1]!=GROUND){
     if(pole[move_player.x][move_player.y-1]==BG){
     move_player.y--;
     correct_move = 1;
     fall();
     dead--;
     pole[move_player.x][move_player.y] = PLAYER;
     pole[player.x][player.y] = BG; 
     }}}
     else{
     if(pole[move_player.x][move_player.y]!=GROUND){
     if(pole[move_player.x][move_player.y+1]!=PLAYER){
     if(pole[move_player.x][move_player.y]!=END){
     if(pole[move_player.x][move_player.y]==CUBE)
     push();
     if(pole[move_player.x][move_player.y]==BOMB)
     push();
     if(!stop){
     fall();
     pole[move_player.x][move_player.y] = PLAYER;
     pole[player.x][player.y] = BG;
     if(!correct_move)
     correct_move = 1;
     }}}}}}}
     if(dead>2){
     MessageBox(NULL, TEXT("Zemrels. Zkus to znovu"), TEXT("Message Box"), 0);
     load(actual_level);
     }  
     if(pole[move_player.x][move_player.y]==END){
     MessageBox(NULL, TEXT("Gratuluji. Postupujete do dalsiho levelu"), TEXT("Message Box"), 0);
     load(actual_level+1);
     }
     dead = 0;
}

void fall(void){
     if(pole[move_player.x][move_player.y+1]==BG){
     move_player.y++;
     correct_move = 1;
     dead++;
     fall();
     }
}

void push(void){
     move_cube.x = move_player.x + direction[direct][0];
     move_cube.y = move_player.y + direction[direct][1];
     if(move_cube.x >= 0 && move_cube.x < X_BLOCKS){
     if(move_cube.y >= 0 && move_cube.y < Y_BLOCKS){
     if(pole[move_cube.x][move_cube.y]==BG){
     if(pole[move_player.x][move_player.y-1]!=CUBE){
     if(pole[move_player.x][move_player.y-1]!=BOMB){

     pole[move_cube.x][move_cube.y] = pole[move_player.x][move_player.y];
     pole[move_player.x][move_player.y] = BG;
     moved_cube = 1;
     }
     else
     stop = 1;
     }
     else
     stop = 1;
     }
     else
     stop = 1;
     }
     else
     stop = 1;
     }
     else
     stop = 1;
}



void search_bomb(){
int i, j;
int znak;
for(i=0; i<X_BLOCKS; i++){
  for(j=0; j<Y_BLOCKS; j++){
    if(pole[i][j]==BOMB){
      znak = pole[i][j];
      around_bomb(i, j, znak);}
  }
}
}

void around_bomb(int i, int j, int znak){
int a;
int k, l;
for(a=0; a<4; a++){
  if(pole[i+direction[a][0]][j+direction[a][1]]==znak){
    pole[i][j] = BG;
    pole[i+direction[a][0]][j+direction[a][1]] = BG;
    refresh = 1;
  }
}
}

void search_fall(){
int i, j;
int znak;
for(i=0; i<X_BLOCKS; i++){
  for(j=0; j<Y_BLOCKS; j++){
    if(pole[i][j]==BOMB){
      znak = pole[i][j];
      around_fall(i, j);}
    if(pole[i][j]==CUBE){
      znak = pole[i][j];
      around_fall(i, j);}
  }
}
}

void around_fall(int i, int j){
if(pole[i][j+1]==BG){
    pole[i][j+1] = pole[i][j];
    pole[i][j] = BG;
    search_bomb();
    around_fall(i, j+1);
    refresh = 1;
    search_fall();
}

}
