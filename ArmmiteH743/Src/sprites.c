/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

sprite.c

Does the basic LCD display commands and drawing in MMBasic.

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
int layer_in_use[MAXLAYER+1];
unsigned char LIFO[MAXBLITBUF];
unsigned char zeroLIFO[MAXBLITBUF];
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)
#ifdef STM32F4version
	int8_t blitmemory[1024*10];
#endif
int LIFOpointer=0;
int zeroLIFOpointer=0;
int sprites_in_use=0;
char *COLLISIONInterrupt=NULL;
int CollisionFound=false;
int sprite_which_collided = -1;
struct blitbuffer blitbuff[MAXBLITBUF];											//Buffer pointers for the BLIT command
extern void Display_Refresh(void);
#if defined(largechip)
static const unsigned char BitReverseTable256[256] = 
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};
uint32_t wordreverse(uint32_t v){
    return (BitReverseTable256[v & 0xff] << 24) | 
    (BitReverseTable256[(v >> 8) & 0xff] << 16) | 
    (BitReverseTable256[(v >> 16) & 0xff] << 8) |
    (BitReverseTable256[(v >> 24) & 0xff]);
}
void arrayreverse(int nbr, uint32_t *p){
    int i, carry=0, xmod, t, nwords=RoundUptoInt(nbr)>>5;
    uint32_t l, k;
    t=nbr & 0b11111;
    xmod= 32 - t;
    for(i=0; i < nwords; i++)p[i]=wordreverse(p[i]);
    for(i=0; i < nwords/2; i++){
       l=p[i];
       p[i]=p[nwords-i-1];
       p[nwords-i-1]=l;
    }
    for(i=nwords-1; i >=0; i--){
        l = p[i];
        k=l & rmask[xmod];
        l = l << xmod;
        l |= carry;
        carry=k>>t;
        p[i]= l;
    }
}
#endif
#ifdef STM32F767xx
void DrawBitmapBuffFast8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){

}
void DrawBufferBuffFast8(int x1, int y1, int x2, int y2, char* p){

}
void ReadBufferBuffFast8(int x1, int y1, int x2, int y2, char* p){

}
void DrawRectangleBuffFast8(int x1, int y1, int x2, int y2, int c){

}
#endif
#ifndef STM32F4version
void flipbuffer(void){
#if defined(largechip)
    if(showbuffer==0)showbuffer=1;
    else showbuffer=0;
#endif
}
#endif
void LIFOadd(int n){
    int i, j=0;
    for(i=0; i<LIFOpointer; i++){
        if(LIFO[i]!=n){
            LIFO[j]=LIFO[i];
            j++;
        }
    }
    LIFO[j]=n;
    LIFOpointer=j+1;
}
void LIFOremove(int n){
    int i, j=0;
    for(i=0; i<LIFOpointer; i++){
        if(LIFO[i]!=n){
            LIFO[j]=LIFO[i];
            j++;
        }
    }
    LIFOpointer=j;
}
void LIFOswap(int n, int m){
    int i;
    for(i=0; i<LIFOpointer; i++){
        if(LIFO[i]==n)LIFO[i]=m;
    }
}
void zeroLIFOadd(int n){
    int i, j=0;
    for(i=0; i<zeroLIFOpointer; i++){
        if(zeroLIFO[i]!=n){
            zeroLIFO[j]=zeroLIFO[i];
            j++;
        }
    }
    zeroLIFO[j]=n;
    zeroLIFOpointer=j+1;
}
void zeroLIFOremove(int n){
    int i, j=0;
    for(i=0; i<zeroLIFOpointer; i++){
        if(zeroLIFO[i]!=n){
            zeroLIFO[j]=zeroLIFO[i];
            j++;
        }
    }
    zeroLIFOpointer=j;
}
void zeroLIFOswap(int n, int m){
    int i;
    for(i=0; i<zeroLIFOpointer; i++){
        if(zeroLIFO[i]==n)zeroLIFO[i]=m;
    }
}
void closeallsprites(void){
    int i;
    for(i = 0; i < MAXBLITBUF; i++) {
        if(i<=MAXLAYER)layer_in_use[i]=0;
        if(blitbuff[i].blitbuffptr!=NULL)FreeMemory(blitbuff[i].blitbuffptr);
        if(blitbuff[i].collisions!=NULL)FreeMemory(blitbuff[i].collisions);
        if(blitbuff[i].blitshowptr!=NULL)FreeMemory(blitbuff[i].blitshowptr);
        if(blitbuff[i].mirror!=NULL)FreeMemory(blitbuff[i].mirror);
        blitbuff[i].blitbuffptr = NULL; 
        blitbuff[i].blitshowptr = NULL; 
        blitbuff[i].collisions = NULL; 
        blitbuff[i].mirror = NULL; 
        blitbuff[i].master=-1;
        blitbuff[i].mymaster=-1;
        blitbuff[i].x=10000;
        blitbuff[i].y=10000;
        blitbuff[i].w=0;
        blitbuff[i].h=0;
        blitbuff[i].next_x = 10000;
        blitbuff[i].next_y = 10000;
        blitbuff[i].bc=0;
        blitbuff[i].layer=-1;
    }
    LIFOpointer=0;
    zeroLIFOpointer=0;
    sprites_in_use=0;
}
void fun_sprite(void){
    int bnbr=0, w=-1, h=-1,t=0, x=10000, y=10000, l=0, n, c=0;
    getargs(&ep, 5,",");
    if((void *)ReadBuffer == (void *)DisplayNotSet) error("Invalid display type");
    if(checkstring(argv[0], "W")) t=1;
    else if(checkstring(argv[0], "H")) t=2;
    else if(checkstring(argv[0], "X")) t=3;
    else if(checkstring(argv[0], "Y")) t=4;
    else if(checkstring(argv[0], "L")) t=5;
    else if(checkstring(argv[0], "C")) t=6;
    else if(checkstring(argv[0], "N")) t=7;
    else if(checkstring(argv[0], "S")) t=8;
    else error("Syntax");
    if(t<7){
        if(argc<3)error("Syntax");
        if(*argv[2] == '#') argv[2]++;
        bnbr = getint(argv[2],0,MAXBLITBUF-1);									// get the number
        if(bnbr==0 && blitbuff[0].collisions!=NULL){
            if(argc==5){
                n=getint(argv[4],1,blitbuff[0].collisions[0]);
                c=blitbuff[0].collisions[n];
            } else c=blitbuff[0].collisions[0];
        }
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            w=blitbuff[bnbr].w;
            h=blitbuff[bnbr].h;
        } 
        if(blitbuff[bnbr].blitshowptr!=NULL){
            x=blitbuff[bnbr].x;
            y=blitbuff[bnbr].y;
            l=blitbuff[bnbr].layer;
            if(blitbuff[bnbr].collisions!=NULL){
                if(argc==5){
                    n=getint(argv[4],1,blitbuff[bnbr].collisions[0]);
                    c=blitbuff[bnbr].collisions[n];
                } else c=blitbuff[bnbr].collisions[0];
            }
        }
    }
    if(t==1)iret=w;
    else if(t==2)iret=h;
    else if(t==3) {if(blitbuff[bnbr].blitshowptr!=NULL)iret=x; else iret=-1;}
    else if(t==4) {if(blitbuff[bnbr].blitshowptr!=NULL)iret=y; else iret=-1;}
    else if(t==5) {if(blitbuff[bnbr].blitshowptr!=NULL)iret=l; else iret=-1;}
    else if(t==6) {if(blitbuff[bnbr].collisions!=NULL)iret=c; else iret=-1;}
    else if(t==7) {
        if(argc==3){
            n=getint(argv[2],0,MAXLAYER);
            iret=layer_in_use[n];
        } else iret=sprites_in_use;
    } else if(t==8) iret = sprite_which_collided;
    else {
    }
    targ = T_INT;   
}
void checklimits(int bnbr, int *n){
        if(blitbuff[bnbr].x<0){
            if(blitbuff[bnbr].collisions==NULL){blitbuff[bnbr].collisions=GetMemory(256);}
            blitbuff[bnbr].collisions[*n]=100;
            (*n)++;
        }
        if(blitbuff[bnbr].y<0){
            if(blitbuff[bnbr].collisions==NULL){blitbuff[bnbr].collisions=GetMemory(256);}
            blitbuff[bnbr].collisions[*n]=101;
            (*n)++;
        }
        if(blitbuff[bnbr].x + blitbuff[bnbr].w > HRes){
            if(blitbuff[bnbr].collisions==NULL){blitbuff[bnbr].collisions=GetMemory(256);}
            blitbuff[bnbr].collisions[*n]=102;
            (*n)++;
        }
        if(blitbuff[bnbr].y + blitbuff[bnbr].h > VRes){
            if(blitbuff[bnbr].collisions==NULL){blitbuff[bnbr].collisions=GetMemory(256);}
            blitbuff[bnbr].collisions[*n]=103;
            (*n)++;
        }
}

void ProcessCollisions(int bnbr){
    int k, j=1, n=1, bcol=1;
    //We know that any collision is caused by movement of sprite bnbr
    // a value of zero indicates that we are processing movement of layer 0 and any
    // sprites on that layer
    CollisionFound=false;
    sprite_which_collided=-1;
    if(blitbuff[bnbr].collisions!=NULL){
        FreeMemory(blitbuff[bnbr].collisions); //clean up any previous collision result
        blitbuff[bnbr].collisions=NULL;
    }
    if(bnbr!=0){ // a specific sprite has moved
        if(blitbuff[bnbr].layer!=0){
            if(layer_in_use[blitbuff[bnbr].layer]+layer_in_use[0]>1){ //other sprites in this layer
                for(k=1;k<MAXBLITBUF;k++){
                    if(k == bnbr) continue;
                    if(j == layer_in_use[blitbuff[bnbr].layer]+layer_in_use[0]) break; //nothing left to process
                    if((blitbuff[k].layer == blitbuff[bnbr].layer || blitbuff[k].layer == 0)){
                        j++;
                        if( !(blitbuff[k].x + blitbuff[k].w < blitbuff[bnbr].x ||
                            blitbuff[k].x > blitbuff[bnbr].x+blitbuff[bnbr].w ||
                            blitbuff[k].y + blitbuff[k].h < blitbuff[bnbr].y ||
                            blitbuff[k].y > blitbuff[bnbr].y + blitbuff[bnbr].h)){
                            if(blitbuff[bnbr].collisions==NULL){blitbuff[bnbr].collisions=GetMemory(256);}
                            blitbuff[bnbr].collisions[n++]=k;
                        }
                    }
                }
            }
        } else {
            for(k=1;k<MAXBLITBUF;k++){
                if(j == sprites_in_use) break; //nothing left to process
                if(k == bnbr) continue;
                if(blitbuff[k].blitshowptr !=NULL) j++;
                if( !(blitbuff[k].x + blitbuff[k].w < blitbuff[bnbr].x ||
                        blitbuff[k].x > blitbuff[bnbr].x+blitbuff[bnbr].w ||
                        blitbuff[k].y + blitbuff[k].h < blitbuff[bnbr].y ||
                        blitbuff[k].y > blitbuff[bnbr].y + blitbuff[bnbr].h)){
                        if(blitbuff[bnbr].collisions==NULL){blitbuff[bnbr].collisions=GetMemory(256);}
                        blitbuff[bnbr].collisions[n++]=k;
                }
            }
        }
// now look for collisions with the edge of the screen
        checklimits(bnbr, &n);
        if(n>1){
            CollisionFound=true;
            sprite_which_collided=bnbr;
            blitbuff[bnbr].collisions[0]=n-1;
        }
    } else { //the background layer has moved
        j=0;
        for(k=1;k<MAXBLITBUF;k++){ //loop through all sprites
            n=1;
            int kk, jj=1;
            if(j == sprites_in_use) break; //nothing left to process
            if(blitbuff[k].blitshowptr!=NULL){ //sprite found
                if(blitbuff[k].collisions!=NULL){
                    FreeMemory(blitbuff[k].collisions); //clean up any previous collision result
                    blitbuff[k].collisions=NULL;
                }
                j++;
                if(layer_in_use[blitbuff[k].layer]+layer_in_use[0]>1){ //other sprites in this layer
                    for(kk=1;kk<MAXBLITBUF;kk++){
                        if(kk == k) continue;
                        if(jj == layer_in_use[blitbuff[k].layer]+layer_in_use[0]) break; //nothing left to process
                        if((blitbuff[kk].layer == blitbuff[k].layer || blitbuff[kk].layer == 0)){
                            jj++;
                            if( !(blitbuff[kk].x + blitbuff[kk].w < blitbuff[k].x ||
                                blitbuff[kk].x > blitbuff[k].x+blitbuff[k].w ||
                                blitbuff[kk].y + blitbuff[kk].h < blitbuff[k].y ||
                                blitbuff[kk].y > blitbuff[k].y + blitbuff[k].h)){
                                if(blitbuff[k].collisions==NULL){blitbuff[k].collisions=GetMemory(256);}
                                blitbuff[k].collisions[n++]=kk;
                            }
                        }
                    }
                }
                checklimits(k, &n);
                if(n>1){
                    if(blitbuff[0].collisions==NULL){
                        blitbuff[0].collisions=GetMemory(256);
                    }
                    blitbuff[0].collisions[bcol]=k;
                    bcol++;
                    blitbuff[k].collisions[0]=n-1;
                }
            }
        }
        if(bcol>1){
            CollisionFound=true;
            sprite_which_collided=0;
            blitbuff[0].collisions[0]=bcol-1;
        }
    }
}

void blithide(int bnbr, int free){
    int w, h, x1, y1;
    char *q;
    if(!(blitbuff[bnbr].blitbuffptr==NULL || blitbuff[bnbr].blitshowptr==NULL)){
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        q=blitbuff[bnbr].blitshowptr;
        x1 = blitbuff[bnbr].x;
        y1 = blitbuff[bnbr].y;
#ifndef STM32F4version
        if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) DrawBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,q);
        else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)DrawBufferBuffFast(x1,y1,x1+w-1,y1+h-1,q);
#if defined(largechip)
        else if(Option.DISPLAY_TYPE==VGA) DrawBufferVGApacked(x1,y1,x1+w-1,y1+h-1,(uint32_t *)q);
#endif
        else 
#endif
		DrawBuffer(x1,y1,x1+w-1,y1+h-1,q);
        if(free){
            FreeMemory(blitbuff[bnbr].blitshowptr);
            blitbuff[bnbr].blitshowptr=NULL;
        }
    } 
}
void BlitShow(int bnbr, int x1, int y1, int mode){
    char *current, *r, *rr, *q, *qq;
    int x, y, rotation, linelength;
    rotation = blitbuff[bnbr].rotation;
    union colourmap
    {
        char rgbbytes[4];
        short rgb[2];
        int r;
    } a, b __attribute((unused));
    int w, h;
    if(blitbuff[bnbr].blitbuffptr!=NULL){
        qq = q = blitbuff[bnbr].blitbuffptr;
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        linelength=w*3;
        if(mode!=0){
            if(blitbuff[bnbr].blitshowptr==NULL){
                current = blitbuff[bnbr].blitshowptr = GetMemory(w*h*3);
            } else {
                current = blitbuff[bnbr].blitshowptr;
                DrawBuffer(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1,current);
            }
        } else current = blitbuff[bnbr].blitshowptr;
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(mode!=2)ReadBuffer(x1,y1,x1+w-1,y1+h-1,current);
        // we now have the old screen image stored together with the coordinates
        rr = r = GetMemory(w*h*3);
        a.r=0;
        b.r=0;
        for(y=0; y<h; y++){
            if(rotation<2)qq=q+linelength*y;
            else qq=q+(h-y-1)*linelength;
            if(rotation==1 || rotation==3)qq+=linelength;
            for(x=0; x<w; x++){
                if(rotation==1 || rotation==3){
                    a.rgbbytes[2]=*--qq;
                    a.rgbbytes[1]=*--qq;
                    a.rgbbytes[0]=*--qq;
                } else {
                    a.rgbbytes[0]=*qq++;
                    a.rgbbytes[1]=*qq++;
                    a.rgbbytes[2]=*qq++;
                }
                b.rgbbytes[0]=*current++;
                b.rgbbytes[1]=*current++;
                b.rgbbytes[2]=*current++;
                if(a.r!=blitbuff[bnbr].bc){
                    *r++=a.rgbbytes[0];
                    *r++=a.rgbbytes[1];
                    *r++=a.rgbbytes[2];
                } else {
                    *r++=b.rgbbytes[0];
                    *r++=b.rgbbytes[1];
                    *r++=b.rgbbytes[2];
                }
            }
        }
        DrawBuffer(x1,y1,x1+w-1,y1+h-1,rr);
        FreeMemory(rr);
    }
}
#ifndef STM32F4version
void BlitShowBuff(int bnbr, int x1, int y1, int mode){
    char *current, *r, *rr, *q, *qq;
    int x, y, rotation, linelength;
    rotation = blitbuff[bnbr].rotation;
    union colourmap
    {
        char rgbbytes[4];
        short rgb[2];
        int r;
    } a, b __attribute((unused));
    int w, h;
    if(blitbuff[bnbr].blitbuffptr!=NULL){
        qq = q = blitbuff[bnbr].blitbuffptr;
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        linelength=w*2;
        if(mode!=0){
            if(blitbuff[bnbr].blitshowptr==NULL){
                current = blitbuff[bnbr].blitshowptr = GetMemory(w*h*2);
            } else {
                current = blitbuff[bnbr].blitshowptr;
                DrawBufferBuffFast(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1,current);
            }
        } else current = blitbuff[bnbr].blitshowptr;
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(mode!=2)ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,current);
        // we now have the old screen image stored together with the coordinates
        rr = r = GetMemory(w*h*2);
        a.r=0;
        b.r=0;
        for(y=0; y<h; y++){
            if(rotation<2)qq=q+linelength*y;
            else qq=q+(h-y-1)*linelength;
            if(rotation==1 || rotation==3)qq+=linelength;
            for(x=0; x<w; x++){
                if(rotation==1 || rotation==3){
                    a.rgbbytes[1]=*--qq;
                    a.rgbbytes[0]=*--qq;
                } else {
                    a.rgbbytes[0]=*qq++;
                    a.rgbbytes[1]=*qq++;
                }
                b.rgbbytes[0]=*current++;
                b.rgbbytes[1]=*current++;
                if(a.r!=blitbuff[bnbr].bc){
                    *r++=a.rgbbytes[0];
                    *r++=a.rgbbytes[1];
                } else {
                    *r++=b.rgbbytes[0];
                    *r++=b.rgbbytes[1];
                }
            }
        }
        DrawBufferBuffFast(x1,y1,x1+w-1,y1+h-1,rr);
        FreeMemory(rr);
    }
}
void BlitShowBuff8(int bnbr, int x1, int y1, int mode){
    char *current, *r, *rr, *q, *qq;
    int x, y, rotation, linelength;
    rotation = blitbuff[bnbr].rotation;
    union colourmap
    {
        char rgbbytes[4];
        short rgb[2];
        int r;
    } a, b __attribute((unused));
    int w, h;
    if(blitbuff[bnbr].blitbuffptr!=NULL){
        qq = q = blitbuff[bnbr].blitbuffptr;
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        linelength=w;
        if(mode!=0){
            if(blitbuff[bnbr].blitshowptr==NULL){
                current = blitbuff[bnbr].blitshowptr = GetMemory(w*h);
            } else {
                current = blitbuff[bnbr].blitshowptr;
                DrawBufferBuffFast8(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1,current);
            }
        } else current = blitbuff[bnbr].blitshowptr;
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(mode!=2)ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,current);
        // we now have the old screen image stored together with the coordinates
        rr = r = GetMemory(w*h);
        a.r=0;
        b.r=0;
        for(y=0; y<h; y++){
            if(rotation<2)qq=q+linelength*y;
            else qq=q+(h-y-1)*linelength;
            if(rotation==1 || rotation==3)qq+=linelength;
            for(x=0; x<w; x++){
                if(rotation==1 || rotation==3){
                    a.rgbbytes[0]=*--qq;
                } else {
                    a.rgbbytes[0]=*qq++;
                }
                b.rgbbytes[0]=*current++;
                if(a.r!=blitbuff[bnbr].bc){
                    *r++=a.rgbbytes[0];

                } else {
                    *r++=b.rgbbytes[0];
                }
            }
        }
        DrawBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,rr);
        FreeMemory(rr);
    }
}
#endif
#if defined largechip
void BlitShowVGA (int bnbr, int x1, int y1, int mode) {
    uint32_t *current, *new, *flip=NULL, linesize;
    int w, h, y, word, i, rotation;
    rotation = blitbuff[bnbr].rotation;
    mode &= 0xF;
    if(blitbuff[bnbr].blitbuffptr!=NULL){
        w=blitbuff[bnbr].w;
        linesize=RoundUptoInt(w)/32;
        h=blitbuff[bnbr].h;
        if(mode!=0){
            if(blitbuff[bnbr].blitshowptr==NULL){
                current = (uint32_t *)GetMemory(((RoundUptoInt(w)*h)/8)*3+12);
                blitbuff[bnbr].blitshowptr = (char *)current;
            } else {
                current = (uint32_t *)blitbuff[bnbr].blitshowptr;
                DrawBufferVGApacked(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1, current);
                current=(uint32_t *)blitbuff[bnbr].blitshowptr;
            } 
        } else current = (uint32_t *)blitbuff[bnbr].blitshowptr;
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(mode!=2)ReadBufferVGApacked(x1,y1,x1+w-1,y1+h-1,current, 1);
        // we now have the old screen image stored together with the coordinates
        uint32_t *r1, *b1, *g1;
        uint32_t *r2, *b2, *g2;
        uint32_t *r3, *b3, *g3;
        new = GetMemory(((RoundUptoInt(w)*h)/8)*3+12);
        uint32_t bitnum, offset;
        int n;
        char rmask=0, gmask=0, bmask=0;
        if(blitbuff[bnbr].bc & 0xFF0000)rmask=1;
        if(blitbuff[bnbr].bc & 0xFF00)gmask=1;
        if(blitbuff[bnbr].bc & 0xFF)bmask=1;
        offset=linesize * h + 1;
        r1=(uint32_t *)blitbuff[bnbr].blitbuffptr;
        if(rotation == 1 || rotation == 3){ //need to mirror
            if(blitbuff[bnbr].mirror==NULL){
                flip=GetMemory(((RoundUptoInt(w)*h)/8)*3+12);
                r2=flip;
                for(i=0; i< (((RoundUptoInt(w)*h)/8)*3+12)>>2; i++)*r2++=*r1++;
            } else flip = (uint32_t *)blitbuff[bnbr].mirror;
        } else {
            flip = (uint32_t *)blitbuff[bnbr].blitbuffptr;
            if(blitbuff[bnbr].mirror!=NULL){
                FreeMemory(blitbuff[bnbr].mirror);
                blitbuff[bnbr].mirror=NULL;
            }
        }
        if(rotation >= 2){
            r1=flip+offset-linesize-1;
        } else {
            r1=flip;
        }
        r2=current;
        r3= new;
        g1=r1+offset; b1=g1+offset;
        g2=r2+offset; b2=g2+offset;
        g3=r3+offset; b3=g3+offset;
        for(y=0; y<h; y++){
            n=w;
            // only reverse the bits if the mirror buffer is not previously created
            if((rotation == 1 || rotation == 3) && blitbuff[bnbr].mirror==NULL){
                arrayreverse(w,r1);
                arrayreverse(w,g1);
                arrayreverse(w,b1);
            }
            for(word=0; word<=RoundUptoInt(w)/32; word++){
                 for(i=31; i>=0; i--){
                    bitnum=1<<i;
                    if(((r1[word] & bitnum) != (rmask << i)) // input same as mask so use original
                    || ((g1[word] & bitnum) != (gmask << i))
                    || ((b1[word] & bitnum) != (bmask << i))){
                        if(r1[word] & bitnum)r3[word] |= bitnum;
                        else r3[word] &= (~bitnum);
                        if(g1[word] & bitnum)g3[word] |= bitnum;
                        else g3[word] &= (~bitnum);
                        if(b1[word] & bitnum)b3[word] |= bitnum;
                        else b3[word] &= (~bitnum);
                    } else {
                        if(r2[word] & bitnum)r3[word] |= bitnum;
                        else r3[word] &= (~bitnum);
                        if(g2[word] & bitnum)g3[word] |= bitnum;
                        else g3[word] &= (~bitnum);
                        if(b2[word] & bitnum)b3[word] |= bitnum;
                        else b3[word] &= (~bitnum);
                    }
                if(--n == 0) break;
                }
            }
            if(rotation >= 2){
                r1-=linesize;g1-=linesize;b1-=linesize;
            } else {
                r1+=linesize;g1+=linesize;b1+=linesize;
            }
            r2+=linesize;g2+=linesize;b2+=linesize;
            r3+=linesize;g3+=linesize;b3+=linesize;
        }
        if(rotation == 1 || rotation == 3)blitbuff[bnbr].mirror=(char *)flip;
        DrawBufferVGApacked(x1,y1,x1+w-1,y1+h-1,new);
        FreeMemory(new);
    }
}

void DoBlitVGA(int x1, int y1, int x2, int y2, int w, int h){
    int max_x;
    uint32_t *buff;
    int memory= FreeSpaceOnHeap()/2; 
    if(((RoundUptoInt(w)*h)/8 *3) > memory- PAGESIZE){ //need to use alternative copy
        if(x1 >=x2) {
            max_x=(memory-PAGESIZE)/h/3;
            buff=GetMemory(((RoundUptoInt(max_x)*h)/8)*3+12);
            while(w > max_x){
                ReadBufferVGApacked(x1,y1,x1+max_x-1,y1+h-1,buff, 1);
                DrawBufferVGApacked(x2,y2,x2+max_x-1,y2+h-1,buff);
                x1+=max_x;
                x2+=max_x;
                w-=max_x;
            }
            ReadBufferVGApacked(x1,y1,x1+w-1,y1+h-1,buff, 0);
            DrawBufferVGApacked(x2,y2,x2+w-1,y2+h-1,buff);
            FreeMemory(buff);
            return;
        }
        else {
            int start_x1,start_x2;
            max_x=(memory-PAGESIZE)/h/3;
            buff=GetMemory(((RoundUptoInt(max_x)*h)/8)*3+12);
            start_x1=x1+w-max_x;
            start_x2=x2+w-max_x;
            while(w > max_x){
                ReadBufferVGApacked(start_x1,y1,start_x1+max_x-1,y1+h-1,buff, 1);
                DrawBufferVGApacked(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
                w-=max_x;
                start_x1-=max_x;
                start_x2-=max_x;
            }
            ReadBufferVGApacked(x1,y1,x1+w-1,y1+h-1,buff, 0);
            DrawBufferVGApacked(x2,y2,x2+w-1,y2+h-1,buff);
            FreeMemory(buff);
            return;
        }
    } else {
        buff=GetMemory(((RoundUptoInt(w)*h)/8)*3+12);
        ReadBufferVGApacked(x1,y1,x1+w-1,y1+h-1,buff, 1);
        DrawBufferVGApacked(x2,y2,x2+w-1,y2+h-1,buff);
        FreeMemory(buff);
    }    
}
#endif

/*
void DoBlitORG(int x1, int y1, int x2, int y2, int w, int h){
    int max_x;
    char *buff;
    int multiplier=3;
#ifndef STM32F4version
    if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16) multiplier=2;
    if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) multiplier=1;
    multiplier=3;
    int memory= FreeSpaceOnHeap()/2; 
#else
    int memory=sizeof(blitmemory);
#endif
    if((w*h*multiplier)>memory-RAMPAGESIZE){ //need to use alternative copy
        if(x1 >=x2) {
            max_x=(memory-RAMPAGESIZE)/h/multiplier;
#ifndef STM32F4version
            buff=GetMemory(max_x*h*multiplier);
#else
            buff=blitmemory;
#endif
            while(w > max_x){
#ifndef STM32F4version
            	if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
            	   ReadBufferBuffFast8(x1,y1,x1+max_x-1,y1+h-1,buff);
            	   DrawBufferBuffFast8(x2,y2,x2+max_x-1,y2+h-1,buff);
            	} else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                   ReadBufferBuffFast(x1,y1,x1+max_x-1,y1+h-1,buff);
                   DrawBufferBuffFast(x2,y2,x2+max_x-1,y2+h-1,buff);
                } else {
                    ReadBuffer(x1,y1,x1+max_x-1,y1+h-1,buff);
                    DrawBuffer(x2,y2,x2+max_x-1,y2+h-1,buff);
                }
#else
                ReadBuffer(x1,y1,x1+max_x-1,y1+h-1,buff);
                DrawBuffer(x2,y2,x2+max_x-1,y2+h-1,buff);
#endif
                x1+=max_x;
                x2+=max_x;
                w-=max_x;
            }
#ifndef STM32F4version
            if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
                ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast8(x2,y2,x2+w-1,y2+h-1,buff);
            } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast(x2,y2,x2+w-1,y2+h-1,buff);
            } else {
                ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
            }
#else
            ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
#endif
            FreeMemory(buff);
            return;
        } else {
            int start_x1,start_x2;
            max_x=(memory-RAMPAGESIZE)/h/multiplier;
#ifndef STM32F4version
            buff=GetMemory(max_x*h*multiplier);
#else
            buff=blitmemory;
#endif
            start_x1=x1+w-max_x;
            start_x2=x2+w-max_x;
            while(w > max_x){
#ifndef STM32F4version
            	if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
            	    ReadBufferBuffFast8(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
            	    DrawBufferBuffFast8(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
            	} else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                    ReadBufferBuffFast(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
                    DrawBufferBuffFast(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
                } else {
                    ReadBuffer(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
                    DrawBuffer(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
                }
#else
                ReadBuffer(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
                DrawBuffer(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
#endif
                w-=max_x;
                start_x1-=max_x;
                start_x2-=max_x;
            }
#ifndef STM32F4version
            if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
                ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast8(x2,y2,x2+w-1,y2+h-1,buff);
            } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast(x2,y2,x2+w-1,y2+h-1,buff);
            } else {
                ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
            }
#else
            ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
#endif
            FreeMemory(buff);
            return;
        }
    } else {
#ifndef STM32F4version
        buff=GetMemory(w*h*multiplier);
        if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
            ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBufferBuffFast8(x2,y2,x2+w-1,y2+h-1,buff);
        } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
            ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBufferBuffFast(x2,y2,x2+w-1,y2+h-1,buff);
        } else {
            ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
        }
#else
        buff=blitmemory;
        ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
        DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
#endif
        FreeMemory(buff);
    }    
}
*/

/* blitmemory is reserved for LCDScroll on IPS_4_16 Display (uses DoBlit to scroll) when
 * EDIT command is used with OPTION LCDPANEL CONSOLE. The editor take all/most of the available
 * memory so FreeSpaceOnHeap()/2 gives nothing. In this case the reserved blitmemeory is used, otherwise the
 * the larger memory block available from FreeSpaceOnHeap is used.
 */
char blitmemory[1024*10];
void DoBlit(int x1, int y1, int x2, int y2, int w, int h){
    int max_x;
    char *buff;
    int multiplier=3;

    if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16) multiplier=2;
    if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) multiplier=1;
    multiplier=3;    //Check this!!
    int bmemory=sizeof(blitmemory);
    int memory= FreeSpaceOnHeap()/2;

    if((w*h*multiplier)>memory-RAMPAGESIZE){ // Not enough memory to do in one go so loop through it.
    	if(x1 >=x2) {
    	   if (memory>bmemory){
    		    max_x=(memory-RAMPAGESIZE)/h/multiplier;
    		    buff=GetMemory(max_x*h*multiplier);
    	   }else{
    	      	max_x=(bmemory)/h/multiplier;
    	       	buff=blitmemory;
    	   }
            while(w > max_x){

            	if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
            	   ReadBufferBuffFast8(x1,y1,x1+max_x-1,y1+h-1,buff);
            	   DrawBufferBuffFast8(x2,y2,x2+max_x-1,y2+h-1,buff);
            	} else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                   ReadBufferBuffFast(x1,y1,x1+max_x-1,y1+h-1,buff);
                   DrawBufferBuffFast(x2,y2,x2+max_x-1,y2+h-1,buff);
                } else {
                    ReadBuffer(x1,y1,x1+max_x-1,y1+h-1,buff);
                    DrawBuffer(x2,y2,x2+max_x-1,y2+h-1,buff);
                }

                x1+=max_x;
                x2+=max_x;
                w-=max_x;
            }

            if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
                ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast8(x2,y2,x2+w-1,y2+h-1,buff);
            } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast(x2,y2,x2+w-1,y2+h-1,buff);
            } else {
                ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
            }

            FreeMemory(buff);
            return;
        } else {   //x2>x1
           int start_x1,start_x2;
     	   if (memory>bmemory){
     		    max_x=(memory-RAMPAGESIZE)/h/multiplier;
     		    buff=GetMemory(max_x*h*multiplier);
     	   }else{
     	      	max_x=(bmemory)/h/multiplier;
     	       	buff=blitmemory;
     	   }
            start_x1=x1+w-max_x;
            start_x2=x2+w-max_x;
            while(w > max_x){

            	if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
            	    ReadBufferBuffFast8(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
            	    DrawBufferBuffFast8(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
            	} else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                    ReadBufferBuffFast(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
                    DrawBufferBuffFast(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
                } else {
                    ReadBuffer(start_x1,y1,start_x1+max_x-1,y1+h-1,buff);
                    DrawBuffer(start_x2,y2,start_x2+max_x-1,y2+h-1,buff);
                }

                w-=max_x;
                start_x1-=max_x;
                start_x2-=max_x;
            }

            if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
                ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast8(x2,y2,x2+w-1,y2+h-1,buff);
            } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBufferBuffFast(x2,y2,x2+w-1,y2+h-1,buff);
            } else {
                ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
                DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
            }

            FreeMemory(buff);
            return;
        }
    } else {    //Enough  memory to do in one go so do it here
    	buff=GetMemory(w*h*multiplier);
        if(Option.DISPLAY_TYPE>=SSD1963_5_8BIT){
            ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBufferBuffFast8(x2,y2,x2+w-1,y2+h-1,buff);
        } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
            ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBufferBuffFast(x2,y2,x2+w-1,y2+h-1,buff);
        } else {
            ReadBuffer(x1,y1,x1+w-1,y1+h-1,buff);
            DrawBuffer(x2,y2,x2+w-1,y2+h-1,buff);
        }

        FreeMemory(buff);
    }
}



int sumlayer(void){
    int i,j=0;
    for(i=0; i<=MAXLAYER ;i++)j+=layer_in_use[i];
    return j;
}
void loadsprite(char *p){
	upng_t* upng;
    union colourmap
    {
        char rgbbytes[4];
        short rgb[2];
        int r;
    } b __attribute((unused)),c;
    int multiplier=3;
#ifndef STM32F4version
    if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)multiplier=2;
    if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) multiplier=1;
#endif
    int bnbr, i, w, h;
    char *q;
    getargs(&p, 5,",");
    if(*argv[0] == '#') argv[0]++;
    bnbr=getint(argv[0],1,MAXBLITBUF-1);
    if(blitbuff[bnbr].blitbuffptr==NULL){
        blitbuff[bnbr].blitshowptr=NULL; //initialise the save pointer
        p=getCstring(argv[2]);
        upng = upng_new_from_file(p);
        upng_header(upng);
        if(upng_get_width(upng) >100 || upng_get_height(upng) >100){
            upng_free(upng);
            error("Image too large");
        }
        if(!(upng_get_format(upng)==1 || upng_get_format(upng)==3)){
            upng_free(upng);
            error("Invalid format");
        }
        upng_decode(upng);
        w=upng_get_width(upng);
        h=upng_get_height(upng);
#ifdef largechip
        if(Option.DISPLAY_TYPE==VGA)blitbuff[bnbr].blitbuffptr = GetMemory(((RoundUptoInt(w)*h)/8)*3+12);
        else 
#endif
		blitbuff[bnbr].blitbuffptr = GetMemory(w*h*multiplier);
        if(argc==5)blitbuff[bnbr].bc=getint(argv[4],0,0xFFFFFF);
        else blitbuff[bnbr].bc=0;
        blitbuff[bnbr].w=w;
        blitbuff[bnbr].h=h;
        blitbuff[bnbr].master=0;
        blitbuff[bnbr].mymaster=-1;
        blitbuff[bnbr].collisions=NULL;
        blitbuff[bnbr].blitshowptr=NULL;
        blitbuff[bnbr].x=10000;
        blitbuff[bnbr].y=10000;
        blitbuff[bnbr].layer=-1;
        blitbuff[bnbr].next_x = 10000;
        blitbuff[bnbr].next_y = 10000;
        q=blitbuff[bnbr].blitbuffptr;
#ifdef largechip
        if(Option.DISPLAY_TYPE==VGA){
            const unsigned char *qq;
            int offset=((RoundUptoInt(w))/32 * h + 1);
            uint32_t *r3, *rr3, *gg3, *g3, *b3, *bb3;
            r3=(uint32_t *)blitbuff[bnbr].blitbuffptr;
            g3=r3+offset;
            b3=g3+offset;
            int bitnum, bytenum;
            if(!(upng_get_format(upng)==3 || upng_get_format(upng)==1)) error("Unsupported .PNG format");
            qq=upng_get_buffer(upng);
            for(y=0; y<h; y++){
                for(x=0; x<w; x++){
                    c.rgbbytes[3]=*qq++;
                    c.rgbbytes[2]=*qq++;
                    c.rgbbytes[1]=*qq++;
                    if(upng_get_format(upng)==3)qq++;
                    bitnum=1<<((31-x) % 32);
                    bytenum = x >>5 ;
                    rr3=r3+bytenum;
                    gg3=g3+bytenum;
                    bb3=b3+bytenum;
                    if(c.rgbbytes[3]>0x78)*rr3 |= bitnum;
                    else *rr3 &= (~bitnum);
                    if(c.rgbbytes[2]>0x78)*gg3 |= bitnum;
                    else *gg3 &= (~bitnum);
                    if(c.rgbbytes[1]>0x78)*bb3 |= bitnum;
                    else *bb3 &= (~bitnum);
                }
            r3+=(RoundUptoInt(w))/32;
            g3+=(RoundUptoInt(w))/32;
            b3+=(RoundUptoInt(w))/32;
            }
        } else 
#endif
#ifndef STM32F4version
		if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) {
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            c.rgbbytes[3]=0;
            while(i-=4){
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                c.rgbbytes[0]=*rr++;
                if(upng_get_format(upng)==3)rr++;
                *q++ = (((c.r >> 16) & 0b11000000)>>2) | (((c.r >> 8) & 0b11000000)>>4) | ((c.r & 0b11000000)>>6);
            }
        } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            c.rgbbytes[3]=0;
            while(i-=4){
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                c.rgbbytes[0]=*rr++;
                if(upng_get_format(upng)==3)rr++;
                *q++ = (((c.r >> 16) & 0b11111000) | ((c.r >> 13) & 0b00000111));
                *q++ = (((c.r >> 5) & 0b11100000) | ((c.r >> 3) & 0b00011111));
            }
        } else {
#endif
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            while(i-=4){
                c.rgbbytes[3]=*rr++;
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                if(upng_get_format(upng)==3)rr++;
                *q++=c.rgbbytes[1];
                *q++=c.rgbbytes[2];
                *q++=c.rgbbytes[3];
            }
#ifndef STM32F4version
        }
#endif
        upng_free(upng);
    } else error("Buffer already in use");
}
void cmd_blit(void) {
    int x1, y1, x2, y2, w, h, bnbr;
    int multiplier=3;
#ifndef STM32F4version
    if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)multiplier=2;
    if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) multiplier=1;
#endif
    char *p, *q;
    if((void *)ReadBuffer == (void *)DisplayNotSet) error("Invalid display type");
    if((p = checkstring(cmdline, "READ"))) {
        getargs(&p, 11,",");
        if(!(argc == 9 || argc == 11)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        x1 = getinteger(argv[2]);
        y1 = getinteger(argv[4]);
        w = getinteger(argv[6]);
        h = getinteger(argv[8]);
        if(w < 1 || h < 1) return;
        if(argc==11)blitbuff[bnbr].bc=getint(argv[10],0,0xFFFFFF);
        else blitbuff[bnbr].bc=0;
        if(blitbuff[bnbr].blitbuffptr==NULL){
            blitbuff[bnbr].master=0;
            blitbuff[bnbr].mymaster=-1;
            blitbuff[bnbr].w=w;
            blitbuff[bnbr].h=h;
            blitbuff[bnbr].master=0;
            blitbuff[bnbr].mymaster=-1;
            blitbuff[bnbr].collisions=NULL;
            blitbuff[bnbr].blitshowptr=NULL;
            blitbuff[bnbr].x=10000;
            blitbuff[bnbr].y=10000;
            blitbuff[bnbr].layer=-1;
            blitbuff[bnbr].next_x = 10000;
            blitbuff[bnbr].next_y = 10000;
#ifdef largechip
            if(Option.DISPLAY_TYPE==VGA)blitbuff[bnbr].blitbuffptr = GetMemory(((RoundUptoInt(w)*h)/8)*3+12);
            else 
#endif
			blitbuff[bnbr].blitbuffptr = GetMemory(w*h*multiplier);
            q=blitbuff[bnbr].blitbuffptr;
        } else {
            if(blitbuff[bnbr].mymaster != -1) error("Can't read into a copy", bnbr);
            if(blitbuff[bnbr].master >0) error("Copies exist", bnbr);
            if(!(blitbuff[bnbr].w==w && blitbuff[bnbr].h==h))error("Existing buffer is incorrect size");
            q=blitbuff[bnbr].blitbuffptr;
        } 
#ifndef STM32F4version
        if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) ReadBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,q);
        else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)ReadBufferBuffFast(x1,y1,x1+w-1,y1+h-1,q);
#if defined(largechip)
        else if(Option.DISPLAY_TYPE==VGA)ReadBufferVGApacked(x1,y1,x1+w-1,y1+h-1,(uint32_t *)q, 0);
#endif
        else 
#endif
		ReadBuffer(x1,y1,x1+w-1,y1+h-1,q);


    } else if((p = checkstring(cmdline, "COPY"))) {
        int cpy,nbr,c1,n1;
        getargs(&p, 5,",");
        if(argc !=5) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            if(*argv[2] == '#') argv[2]++;
            c1 = cpy = getint(argv[2], 1, MAXBLITBUF-1);
            n1 = nbr = getint(argv[4], 1, MAXBLITBUF-2);

            while(n1) {
                if(blitbuff[c1].blitbuffptr!=NULL)error("Buffer already in use %",c1);
                if(blitbuff[bnbr].master==-1)error("Can't copy a copy");
                n1--;
                c1++;
            }
            while(nbr) {
                blitbuff[cpy].blitbuffptr=blitbuff[bnbr].blitbuffptr;
                blitbuff[cpy].w=blitbuff[bnbr].w;
                blitbuff[cpy].h=blitbuff[bnbr].h;
                blitbuff[cpy].w=blitbuff[bnbr].w;
                blitbuff[cpy].bc=blitbuff[bnbr].bc;
                blitbuff[cpy].blitshowptr = NULL; 
                blitbuff[cpy].collisions=NULL;
                blitbuff[cpy].x=10000;
                blitbuff[cpy].y=10000;
                blitbuff[cpy].next_x = 10000;
                blitbuff[cpy].next_y = 10000;
                blitbuff[cpy].layer=-1;
                blitbuff[cpy].mymaster=bnbr;
                blitbuff[cpy].master=-1;
                blitbuff[bnbr].master |= (1<<cpy);
                nbr--;
                cpy++;
            }
        } else error("Buffer not in use");

    } else if((p = checkstring(cmdline, "LOAD"))) {
        loadsprite(p);

    } else if((p = checkstring(cmdline, "INTERRUPT"))) {
        getargs(&p, 1,",");
        COLLISIONInterrupt = GetIntAddress(argv[0]);					// get the interrupt location
        InterruptUsed = true;
        return;

    } else if((p = checkstring(cmdline, "NOINTERRUPT"))) {
        COLLISIONInterrupt = NULL;					// get the interrupt location
        return;

    } else if((p = checkstring(cmdline, "CLOSE ALL"))) {
        closeallsprites();
        
    } else if((p = checkstring(cmdline, "HIDE"))) {
        getargs(&p, 1,",");
        if(argc !=1) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            if(blitbuff[bnbr].collisions!=NULL)FreeMemory(blitbuff[bnbr].collisions);
            if(blitbuff[bnbr].mirror!=NULL)FreeMemory(blitbuff[bnbr].mirror);
            if(blitbuff[bnbr].blitshowptr!=NULL){
                sprites_in_use--;
#if defined largechip
                if(Option.DISPLAY_TYPE==VGA) VGAbuffcopy();
#endif
                blithide(bnbr, 1);
                layer_in_use[blitbuff[bnbr].layer]--;
                blitbuff[bnbr].x=10000;
                blitbuff[bnbr].y=10000;
                if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
                else LIFOremove(bnbr);
                blitbuff[bnbr].layer=-1;
                blitbuff[bnbr].next_x = 10000;
                blitbuff[bnbr].next_y = 10000;

#if defined largechip
                if(Option.DISPLAY_TYPE==VGA)  flipbuffer();
#endif
#ifndef STM32F4version
                if(Option.Refresh)Display_Refresh();
#endif
            } else error("Not Showing");
        } else error("Buffer not in use");
        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
//
    } else if((p = checkstring(cmdline, "NEXT"))) {
        getargs(&p, 5,",");
        if(!(argc ==5)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);									// get the number
        blitbuff[bnbr].next_x = getint(argv[2],-blitbuff[bnbr].w+1,HRes-1);
        blitbuff[bnbr].next_y = getint(argv[4],-blitbuff[bnbr].h+1,VRes-1);
//
    } else if((p = checkstring(cmdline, "SHOW"))) {
        int layer;
        getargs(&p, 9,",");
        if(!(argc ==7 || argc==9)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){ 
            x1 = getint(argv[2],-blitbuff[bnbr].w+1,HRes-1);
            y1 = getint(argv[4],-blitbuff[bnbr].h+1,VRes-1);
            layer=getint(argv[6],0,MAXLAYER);
            if(argc==9)blitbuff[bnbr].rotation=getint(argv[8],0,3);
            else blitbuff[bnbr].rotation = 0;
#if defined largechip
            if(Option.DISPLAY_TYPE==VGA) VGAbuffcopy();
#endif
            q=blitbuff[bnbr].blitbuffptr;
            w=blitbuff[bnbr].w;
            h=blitbuff[bnbr].h;
            if(blitbuff[bnbr].blitshowptr!=NULL){
                layer_in_use[blitbuff[bnbr].layer]--;
                if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
                else LIFOremove(bnbr);
                sprites_in_use--;
            }
            blitbuff[bnbr].layer=layer;
            layer_in_use[blitbuff[bnbr].layer]++;
            if(blitbuff[bnbr].layer==0) zeroLIFOadd(bnbr);
            else LIFOadd(bnbr);
            if(blitbuff[bnbr].collisions != NULL){
                FreeMemory(blitbuff[bnbr].collisions);
                blitbuff[bnbr].collisions=NULL;
            }
            sprites_in_use++;
#ifndef STM32F4version
            if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(bnbr, x1, y1, 1);
            else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(bnbr, x1, y1, 1);
#if defined(largechip)
            else if(Option.DISPLAY_TYPE==VGA){
                BlitShowVGA(bnbr, x1, y1, 1);
                flipbuffer();
            }  
#endif
            else
#endif
            BlitShow(bnbr, x1, y1, 1);
#ifndef STM32F4version
            if(Option.Refresh)Display_Refresh();
#endif
            ProcessCollisions(bnbr);
            if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
        } else error("Buffer not in use");
    } else if((p = checkstring(cmdline, "WRITE"))) {
        getargs(&p, 5,",");
        if(argc !=5) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            x1 = getint(argv[2], -blitbuff[bnbr].w+1, HRes);
            y1 = getint(argv[4], -blitbuff[bnbr].h+1, VRes);
            q=blitbuff[bnbr].blitbuffptr;
            w=blitbuff[bnbr].w;
            h=blitbuff[bnbr].h;
#ifndef STM32F4version
            if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) DrawBufferBuffFast8(x1,y1,x1+w-1,y1+h-1,q);
            else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)DrawBufferBuffFast(x1,y1,x1+w-1,y1+h-1,q);
#if defined(largechip)
            else if(Option.DISPLAY_TYPE==VGA){
                VGAbuffcopy();
                DrawBufferVGApacked(x1,y1,x1+w-1,y1+h-1,(uint32_t *)q);
                flipbuffer();
             } 
#endif
            else 
#endif
			DrawBuffer(x1,y1,x1+w-1,y1+h-1,q);
#ifndef STM32F4version
            if(Option.Refresh)Display_Refresh();
#endif
        } else error("Buffer not in use");
    } else if((p = checkstring(cmdline, "CLOSE"))) {
        getargs(&p, 1,",");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);
        if(bnbr==0){
            if(blitbuff[bnbr].blitbuffptr!=NULL){
                FreeMemory(blitbuff[bnbr].collisions);
            }
        } else {
            if(blitbuff[bnbr].blitbuffptr!=NULL){
               if(blitbuff[bnbr].master>0) error("Copies still open");
               if(blitbuff[bnbr].mirror!=NULL)FreeMemory(blitbuff[bnbr].mirror);
               if(blitbuff[bnbr].blitshowptr!=NULL){
#if defined largechip
                    if(Option.DISPLAY_TYPE==VGA) VGAbuffcopy();
#endif
                    blithide(bnbr, 1);
                    if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
                    else LIFOremove(bnbr);
                    layer_in_use[blitbuff[bnbr].layer]--;
                    sprites_in_use--;
#ifndef STM32F4version
                    if(Option.Refresh)Display_Refresh();
                    if(Option.DISPLAY_TYPE==VGA)  flipbuffer();
#endif
                }
                if(blitbuff[bnbr].mymaster==-1)FreeMemory(blitbuff[bnbr].blitbuffptr);
                else blitbuff[blitbuff[bnbr].mymaster].master &= ~(1<<bnbr);
                if(blitbuff[bnbr].collisions!=NULL)FreeMemory(blitbuff[bnbr].collisions);
                blitbuff[bnbr].blitbuffptr=NULL;
                blitbuff[bnbr].master=-1;
                blitbuff[bnbr].mymaster=-1;
                blitbuff[bnbr].w=0;
                blitbuff[bnbr].h=0;
                blitbuff[bnbr].bc=0;
                blitbuff[bnbr].x=10000;
                blitbuff[bnbr].y=10000;
                blitbuff[bnbr].layer=-1;
                blitbuff[bnbr].next_x = 10000;
                blitbuff[bnbr].next_y = 10000;
            } else error("Buffer not in use");
         }
         if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
    } else if((p = checkstring(cmdline, "SWAP"))) {
        int rbnbr;
        getargs(&p, 3,",");
        if(!(argc ==3)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],0,MAXBLITBUF-1);									// get the number
        if(*argv[2] == '#') argv[0]++;
        rbnbr = getint(argv[2],0,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr==NULL || blitbuff[bnbr].blitshowptr == NULL) error("Original buffer not displayed");
        if(blitbuff[rbnbr].blitbuffptr==NULL)error("Original buffer not displayed");
        if(blitbuff[rbnbr].blitshowptr != NULL) error("New buffer in use");
        if(!(blitbuff[rbnbr].w ==blitbuff[bnbr].w && blitbuff[rbnbr].h ==blitbuff[bnbr].h)) error("Size mismatch");
// copy the relevant data
        blitbuff[rbnbr].blitshowptr=blitbuff[bnbr].blitshowptr;
        blitbuff[rbnbr].x=blitbuff[bnbr].x;
        blitbuff[rbnbr].y=blitbuff[bnbr].y;
        blitbuff[rbnbr].layer=blitbuff[bnbr].layer;
        if(blitbuff[rbnbr].layer==0)zeroLIFOswap(bnbr,rbnbr);
        else LIFOswap(bnbr,rbnbr);
// "Hide" the old sprite        
        blitbuff[bnbr].blitshowptr=NULL;
        blitbuff[bnbr].x=10000;
        blitbuff[bnbr].y=10000;
        blitbuff[bnbr].layer=-1;
        blitbuff[bnbr].next_x = 10000;
        blitbuff[bnbr].next_y = 10000;
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            FreeMemory(blitbuff[bnbr].collisions);
        }
#ifndef STM32F4version
        if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(rbnbr, blitbuff[rbnbr].x, blitbuff[rbnbr].y, 2);
        else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(rbnbr, blitbuff[rbnbr].x, blitbuff[rbnbr].y, 2);
#if defined(largechip)
        else if(Option.DISPLAY_TYPE==VGA){
            VGAbuffcopy();
            BlitShowVGA(rbnbr, blitbuff[rbnbr].x, blitbuff[rbnbr].y, 2);
            flipbuffer();
        } 
#endif
        else 
#endif
		BlitShow(rbnbr, blitbuff[rbnbr].x, blitbuff[rbnbr].y, 2);
        if(Option.Refresh)Display_Refresh();
        ProcessCollisions(bnbr);
        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
        
        
        
    } else if((p = checkstring(cmdline, "MOVE"))) {
        int i;
#if defined largechip
        if(Option.DISPLAY_TYPE==VGA)VGAbuffcopy();
#endif
        for(i=LIFOpointer-1; i>= 0; i--) blithide(LIFO[i],0);
        for(i=zeroLIFOpointer-1; i>= 0; i--)blithide(zeroLIFO[i],0);
//
        for(i=0; i< zeroLIFOpointer; i++){
            if(blitbuff[zeroLIFO[i]].next_x != 10000){
                blitbuff[zeroLIFO[i]].x=blitbuff[zeroLIFO[i]].next_x;
                blitbuff[zeroLIFO[i]].next_x = 10000;
            }
            if(blitbuff[zeroLIFO[i]].next_y != 10000){
                blitbuff[zeroLIFO[i]].y=blitbuff[zeroLIFO[i]].next_y;
                blitbuff[zeroLIFO[i]].next_y = 10000;
            }
#ifndef STM32F4version
            if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
            else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#if defined largechip
            else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);

#endif
            else
#endif
            BlitShow(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }
        for(i=0; i< LIFOpointer; i++){
            if(blitbuff[LIFO[i]].next_x != 10000){
                blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                blitbuff[LIFO[i]].next_x = 10000;
            }
            if(blitbuff[LIFO[i]].next_y != 10000){
                blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                blitbuff[LIFO[i]].next_y = 10000;
            }
#ifndef STM32F4version
            if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
            else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#if defined largechip
            else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
             
#endif
            else 
#endif
			BlitShow(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);

        }
        ProcessCollisions(0);
#ifndef STM32F4version
        if(Option.Refresh)Display_Refresh();
        if(Option.DISPLAY_TYPE==VGA)flipbuffer();
#endif
    } else if((p = checkstring(cmdline, "SCROLLV"))) {
        int i,n,m=0, blank=-1;
        char *current=NULL;
        getargs(&p, 3,",");
        n = getint(argv[0],-(VRes-1), VRes-1);
        if(argc==3)blank=getint(argv[2],0,0xFFFFFF);
        if(n!=0){
            m=n;if(n<0)m=-n;
#if defined largechip
            if(Option.DISPLAY_TYPE==VGA){
                if(blank<0)current=GetMemory(((RoundUptoInt(HRes)*m)/8)*3+12);
                VGAbuffcopy();
            } else 
#endif
			if(blank<0)current=GetMemory((HRes*m)*multiplier);
            for(i=LIFOpointer-1; i>= 0; i--) blithide(LIFO[i],0);
            for(i=zeroLIFOpointer-1; i>= 0; i--){
                    int ys=blitbuff[zeroLIFO[i]].y+ (blitbuff[zeroLIFO[i]].h >> 1);
                    blithide(zeroLIFO[i],0);
                    ys-=n;
                    if(ys>=VRes)ys-=VRes;
                    if(ys < 0)ys+=VRes;
                    blitbuff[zeroLIFO[i]].y=ys-(blitbuff[zeroLIFO[i]].h >> 1);
            }
#if defined largechip
           if(Option.DISPLAY_TYPE==VGA){
               if(blank<0){
                    if(n>0){
                        ReadBufferVGApacked(0,0,HRes-1,m-1,(uint32_t *)current, 1);
                    } else {
                        ReadBufferVGApacked(0,VRes-m,HRes-1,VRes-1,(uint32_t *)current, 1);
                    }
               }
                ScrollVGAvertical(n);
                if(blank<0){
                    if(n>0){
                        DrawBufferVGApacked(0,VRes-m,HRes-1,VRes-1,(uint32_t *)current);
                    } else {
                        DrawBufferVGApacked(0,0,HRes-1,m-1,(uint32_t *)current);
                    }
                    FreeMemory(current);
                } else {
                    if(n>0){
                        DrawRectangleVGAbackground(0,VRes-m,HRes-1,VRes-1, blank) ;
                    } else {
                        DrawRectangleVGAbackground(0,0,HRes-1,m-1, blank) ;
                    }
                }
           } else 
#endif
#ifndef STM32F4version
			if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) {
               if(n>0){
                   if(blank<0)ReadBufferBuffFast8(0,0,HRes-1,m-1,current);
                   DoBlit(0,m,0,0,HRes,VRes-m);
                   if(blank<0)DrawBufferBuffFast8(0,VRes-m,HRes-1,VRes-1,current);
                   else DrawRectangle(0,VRes-m,HRes-1,VRes-1, blank) ;
               } else {
                   if(blank<0)ReadBufferBuffFast8(0,VRes-m,HRes-1,VRes-1,current);
                   DoBlit(0,0,0,m,HRes,VRes-m);
                   if(blank<0)DrawBufferBuffFast8(0,0,HRes-1,m-1,current);
                   else DrawRectangle(0,0,HRes-1,m-1, blank) ;
               }
               FreeMemory(current);
           } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                if(n>0){
                    if(blank<0)ReadBufferBuffFast(0,0,HRes-1,m-1,current);
                    DoBlit(0,m,0,0,HRes,VRes-m);
                    if(blank<0)DrawBufferBuffFast(0,VRes-m,HRes-1,VRes-1,current);
                    else DrawRectangle(0,VRes-m,HRes-1,VRes-1, blank) ;
                } else {
                    if(blank<0)ReadBufferBuffFast(0,VRes-m,HRes-1,VRes-1,current);
                    DoBlit(0,0,0,m,HRes,VRes-m);
                    if(blank<0)DrawBufferBuffFast(0,0,HRes-1,m-1,current);
                    else DrawRectangle(0,0,HRes-1,m-1, blank) ;
                }
                FreeMemory(current);
            } else {
#endif
                if(n>0){
                    if(blank<0)ReadBuffer(0,0,HRes-1,m-1,current);
                    DoBlit(0,m,0,0,HRes,VRes-m);
                    if(blank<0)DrawBuffer(0,VRes-m,HRes-1,VRes-1,current);
                    else DrawRectangle(0,VRes-m,HRes-1,VRes-1, blank) ;
                } else {
                    if(blank<0)ReadBuffer(0,VRes-m,HRes-1,VRes-1,current);
                    DoBlit(0,0,0,m,HRes,VRes-m);
                    if(blank<0)DrawBuffer(0,0,HRes-1,m-1,current);
                    else DrawRectangle(0,0,HRes-1,m-1, blank) ;
                }
                FreeMemory(current);
#ifndef STM32F4version
            }
#endif
            for(i=0; i< zeroLIFOpointer; i++){
#ifndef STM32F4version
            if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
            else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#if defined largechip
            else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#endif
            else 
#endif
			BlitShow(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
             }
            for(i=0; i< LIFOpointer; i++){
                if(blitbuff[LIFO[i]].next_x != 10000){
                    blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                    blitbuff[LIFO[i]].next_x = 10000;
                }
                if(blitbuff[LIFO[i]].next_y != 10000){
                    blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                    blitbuff[LIFO[i]].next_y = 10000;
                }
#ifndef STM32F4version
                if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
                else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#if defined largechip
                else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#endif
                else 
#endif
				BlitShow(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
            }
        ProcessCollisions(0);
#ifndef STM32F4version
        if(Option.Refresh)Display_Refresh();
        if(Option.DISPLAY_TYPE==VGA)flipbuffer();
#endif
        }
    } else if((p = checkstring(cmdline, "SCROLLR"))) {
        int i,xmin,xmax,ymin,ymax, blank=-1, sx=0, sy=0, b1, b2, xs=10000, ys=10000;
        uint32_t *current1=NULL, *current2=NULL;
        char *current3=NULL, *current4=NULL;
        getargs(&p, 13,",");
        if(!(argc ==11 || (argc==13))) error("Syntax");
        x1 = getint(argv[0],0,HRes-2);
        y1 = getint(argv[2],0,VRes-2);
        w = getint(argv[4],1,HRes-x1);
        h = getint(argv[6],1,VRes-y1);
        sx = getint(argv[8],1-w, w-1);
        sy = getint(argv[10],1-h, h-1);
        if(sx>=0){ //convert coordinates for use with BLIT
            x2=x1+sx;
            w-=sx;
        } else {
            x2=x1;
            x1-=sx; //sx is negative so this subtracts it
            w+=sx;
        }
        if(sy<0){ //convert coordinates for use with BLIT
            y2=y1-sy;
            h+=sy;
        } else {
            y2=y1;
            y1+=sy; 
            h-=sy;
        }
        if(argc==13)blank=getint(argv[12],0,0xFFFFFF);
        xmin=min(x1,x2);
        xmax=max(x1,x2)+w-1;
        ymin=min(y1,y2);
        ymax=max(y1,y2)+h-1;
        if(w < 1 || h < 1) return;
        if(x1 < 0) { x2 -= x1; w += x1; x1 = 0; }
        if(x2 < 0) { x1 -= x2; w += x2; x2 = 0; }
        if(y1 < 0) { y2 -= y1; h += y1; y1 = 0; }
        if(y2 < 0) { y1 -= y2; h += y2; y2 = 0; }
        if(x1 + w > HRes) w = HRes - x1;
        if(x2 + w > HRes) w = HRes - x2;
        if(y1 + h > VRes) h = VRes - y1;
        if(y2 + h > VRes) h = VRes - y2;
        if(w < 1 || h < 1 || x1 < 0 || x1 + w > HRes || x2 < 0 || x2 + w > HRes || y1 < 0 || y1 + h > VRes || y2 < 0 || y2 + h > VRes) return;

        b1=RoundUptoInt(abs(sx))*ymax;
        b2=RoundUptoInt(xmax)*abs(sy);
#if defined largechip
        if(Option.DISPLAY_TYPE==VGA){
            if(blank<0){
                if(x1!=x2)current1=GetMemory((b1/8)*3+12);
                if(y1!=y2)current2=GetMemory((b2/8)*3+12);
            }
            VGAbuffcopy();
        } else 
#endif
		if(blank<0){
            if(x1!=x2)current3=GetMemory(b1*multiplier);
            if(y1!=y2)current4=GetMemory(b2*multiplier);
        }

        for(i=LIFOpointer-1; i>= 0; i--) {
            blithide(LIFO[i],0);
        }
            
        for(i=zeroLIFOpointer-1; i>= 0; i--){
            xs=blitbuff[zeroLIFO[i]].x + (blitbuff[zeroLIFO[i]].w >> 1);
            ys=blitbuff[zeroLIFO[i]].y + (blitbuff[zeroLIFO[i]].h >> 1);
            blithide(zeroLIFO[i],0);
            if((xs <= xmax) && (xs  >= xmin) && (ys <= ymax) && (ys >= ymin)){
                xs+=(x2-x1);
                if(xs>=xmax)xs -=  (xmax-xmin+1);
                if(xs < xmin)xs += (xmax-xmin+1);
                blitbuff[zeroLIFO[i]].x=xs-(blitbuff[zeroLIFO[i]].w >> 1);
                ys+=(y2-y1);
                if(ys>=ymax)ys -=  (ymax-ymin+1);
                if(ys < ymin)ys += (ymax-ymin+1);
                blitbuff[zeroLIFO[i]].y=ys-(blitbuff[zeroLIFO[i]].h >> 1);
            }
        }

#if defined(largechip)
        if(Option.DISPLAY_TYPE == VGA){
            if(blank==-1){
                if(x1!=x2){
                    if(sx>0){
                        ReadBufferVGApacked(xmax-sx+1, ymin, xmax, ymax, current1, 1) ;
                    } else {
                        ReadBufferVGApacked(xmin, ymin, xmin-sx-1, ymax,  current1, 1) ;
                    }
                }
                if(y1!=y2){
                    if(sy>0){
                        ReadBufferVGApacked(xmin, ymin, xmax, ymin-1+sy, current2, 1);
                    } else {
                        ReadBufferVGApacked(xmin, ymax+sy+1, xmax, ymax, current2, 1);
                    }
                }
            }
            DoBlitVGA( x1, y1, x2, y2, w, h);
            if(x1!=x2){
                if(blank!=-1){
                    if(sx>0){
                        DrawRectangleVGAbackground(xmin, ymin, xmin+sx-1, ymax,  blank) ;
                    } else {
                        DrawRectangleVGAbackground(xmax+sx+1, ymin, xmax, ymax, blank) ;
                    }
                } else {
                    if(sx>0){
                        DrawBufferVGApacked(xmin, ymin, xmin+sx-1, ymax,  current1) ;
                    } else {
                        DrawBufferVGApacked(xmax+sx+1, ymin, xmax, ymax, current1) ;
                    }
                }
            }
            if(y1!=y2){
                if(blank!=-1){
                    if(sy>0){
                        DrawRectangleVGAbackground(xmin, ymax-sy+1, xmax, ymax, blank);
                    } else {
                        DrawRectangleVGAbackground(xmin, ymin, xmax, ymin-1-sy, blank);
                    }
                } else {
                    if(sy>0){
                        DrawBufferVGApacked(xmin, ymax-sy+1, xmax, ymax, current2);
                    } else {
                        DrawBufferVGApacked(xmin, ymin, xmax, ymin-1-sy, current2);
                    }
                }
            }
        } else 
#endif
#ifndef STM32F4version
		if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) {
            if(blank==-1){
                if(x1!=x2){
                    if(sx>0){
                        ReadBufferBuffFast8(xmax-sx+1, ymin, xmax, ymax, current3) ;
                    } else {
                        ReadBufferBuffFast8(xmin, ymin, xmin-sx-1, ymax,  current3) ;
                    }
                }
                if(y1!=y2){
                    if(sy>0){
                        ReadBufferBuffFast8(xmin, ymin, xmax, ymin-1+sy, current4);
                    } else {
                        ReadBufferBuffFast8(xmin, ymax+sy+1, xmax, ymax, current4);
                    }
                }
            }
            DoBlit( x1, y1, x2, y2, w, h);
            if(x1!=x2){
                if(blank!=-1){
                    if(sx>0){
                        DrawRectangle(xmin, ymin, xmin+sx-1, ymax,  blank) ;
                    } else {
                        DrawRectangle(xmax+sx+1, ymin, xmax, ymax, blank) ;
                    }
                } else {
                    if(sx>0){
                        DrawBufferBuffFast8(xmin, ymin, xmin+sx-1, ymax,  current3) ;
                    } else {
                        DrawBufferBuffFast8(xmax+sx+1, ymin, xmax, ymax, current3) ;
                    }
                }
            }
            if(y1!=y2){
                if(blank!=-1){
                    if(sy>0){
                        DrawRectangle(xmin, ymax-sy+1, xmax, ymax, blank);
                    } else {
                        DrawRectangle(xmin, ymin, xmax, ymin-1-sy, blank);
                    }
                } else {
                    if(sy>0){
                        DrawBufferBuffFast8(xmin, ymax-sy+1, xmax, ymax, current4);
                    } else {
                        DrawBufferBuffFast8(xmin, ymin, xmax, ymin-1-sy, current4);
                    }
                }
            }
        } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
            if(blank==-1){
                if(x1!=x2){
                    if(sx>0){
                        ReadBufferBuffFast(xmax-sx+1, ymin, xmax, ymax, current3) ;
                    } else {
                        ReadBufferBuffFast(xmin, ymin, xmin-sx-1, ymax,  current3) ;
                    }
                }
                if(y1!=y2){
                    if(sy>0){
                        ReadBufferBuffFast(xmin, ymin, xmax, ymin-1+sy, current4);
                    } else {
                        ReadBufferBuffFast(xmin, ymax+sy+1, xmax, ymax, current4);
                    }
                }
            }
            DoBlit( x1, y1, x2, y2, w, h);
            if(x1!=x2){
                if(blank!=-1){
                    if(sx>0){
                        DrawRectangle(xmin, ymin, xmin+sx-1, ymax,  blank) ;
                    } else {
                        DrawRectangle(xmax+sx+1, ymin, xmax, ymax, blank) ;
                    }
                } else {
                    if(sx>0){
                        DrawBufferBuffFast(xmin, ymin, xmin+sx-1, ymax,  current3) ;
                    } else {
                        DrawBufferBuffFast(xmax+sx+1, ymin, xmax, ymax, current3) ;
                    }
                }
            }
            if(y1!=y2){
                if(blank!=-1){
                    if(sy>0){
                        DrawRectangle(xmin, ymax-sy+1, xmax, ymax, blank);
                    } else {
                        DrawRectangle(xmin, ymin, xmax, ymin-1-sy, blank);
                    }
                } else {
                    if(sy>0){
                        DrawBufferBuffFast(xmin, ymax-sy+1, xmax, ymax, current4);
                    } else {
                        DrawBufferBuffFast(xmin, ymin, xmax, ymin-1-sy, current4);
                    }
                }
            }
        } else {
#endif
            if(blank==-1){
                if(x1!=x2){
                    if(sx>0){
                        ReadBuffer(xmax-sx+1, ymin, xmax, ymax, current3) ;
                    } else {
                        ReadBuffer(xmin, ymin, xmin-sx-1, ymax,  current3) ;
                    }
                }
                if(y1!=y2){
                    if(sy>0){
                        ReadBuffer(xmin, ymin, xmax, ymin-1+sy, current4);
                    } else {
                        ReadBuffer(xmin, ymax+sy+1, xmax, ymax, current4);
                    }
                }
            }
            DoBlit( x1, y1, x2, y2, w, h);
            if(x1!=x2){
                if(blank!=-1){
                    if(sx>0){
                        DrawRectangle(xmin, ymin, xmin+sx-1, ymax,  blank) ;
                    } else {
                        DrawRectangle(xmax+sx+1, ymin, xmax, ymax, blank) ;
                    }
                } else {
                    if(sx>0){
                        DrawBuffer(xmin, ymin, xmin+sx-1, ymax,  current3) ;
                    } else {
                        DrawBuffer(xmax+sx+1, ymin, xmax, ymax, current3) ;
                    }
                }
            }
            if(y1!=y2){
                if(blank!=-1){
                    if(sy>0){
                        DrawRectangle(xmin, ymax-sy+1, xmax, ymax, blank);
                    } else {
                        DrawRectangle(xmin, ymin, xmax, ymin-1-sy, blank);
                    }
                } else {
                    if(sy>0){
                        DrawBuffer(xmin, ymax-sy+1, xmax, ymax, current4);
                    } else {
                        DrawBuffer(xmin, ymin, xmax, ymin-1-sy, current4);
                    }
                }
           }
#ifndef STM32F4version
        }
#endif
        if(current1!=NULL)FreeMemory(current1);
        if(current2!=NULL)FreeMemory(current2);
        if(current3!=NULL)FreeMemory(current3);
        if(current4!=NULL)FreeMemory(current4);
        for(i=0; i< zeroLIFOpointer; i++){
#ifndef STM32F4version
        	if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        	else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#if defined largechip
            else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#endif
            else 
#endif
			BlitShow(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }

        for(i=0; i< LIFOpointer; i++){
            if(blitbuff[LIFO[i]].next_x != 10000){
                blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                blitbuff[LIFO[i]].next_x = 10000;
            }
            if(blitbuff[LIFO[i]].next_y != 10000){
                blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                blitbuff[LIFO[i]].next_y = 10000;
            }
#ifndef STM32F4version
            if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
            else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#if defined largechip
            else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#endif
            else 
#endif
			BlitShow(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
        ProcessCollisions(0);

#ifndef STM32F4version
        if(Option.Refresh)Display_Refresh();
        if(Option.DISPLAY_TYPE==VGA)flipbuffer();
#endif

    } else if((p = checkstring(cmdline, "SCROLLH"))) {
        int i, n, m=0, blank=-1;
        char *current=NULL;
        getargs(&p, 3,",");
        
        n = getint(argv[0],-31, 31);
        if(argc==3)blank=getint(argv[2],0,0xFFFFFF);
        if(n!=0){
            m=n;if(n<0)m=-n;
#if defined largechip
            if(Option.DISPLAY_TYPE==VGA){
                if(blank<0)current=GetMemory(((RoundUptoInt(m)*VRes)/8)*3+12);
                VGAbuffcopy();
            } else 
#endif
			if(blank<0)current=GetMemory((VRes*m)*multiplier);
            for(i=LIFOpointer-1; i>= 0; i--) blithide(LIFO[i],0);
            for(i=zeroLIFOpointer-1; i>= 0; i--){
                    int xs=blitbuff[zeroLIFO[i]].x+ (blitbuff[zeroLIFO[i]].w >> 1);
                    blithide(zeroLIFO[i],0);
                    xs+=n;
                    if(xs>=HRes)xs-=HRes;
                    if(xs < 0)xs+=HRes;
                    blitbuff[zeroLIFO[i]].x=xs-(blitbuff[zeroLIFO[i]].w >> 1);
            }
#if defined largechip
            if(Option.DISPLAY_TYPE==VGA){
                if(blank<0){
                    if(n>0){
                        ReadBufferVGApacked(HRes-m,0,HRes-1,VRes-1,(uint32_t *)current, 1);
                    } else {
                        ReadBufferVGApacked(0,0,m-1,VRes-1,(uint32_t *)current, 1);
                    }
                }
                ScrollVGAsideways(n);
                if(blank<0){
                    if(n>0){
                        DrawBufferVGApacked(0,0,m-1,VRes-1,(uint32_t *)current);
                    } else {
                        DrawBufferVGApacked(HRes-m,0,HRes-1,VRes-1, (uint32_t *)current);
                    }
                    FreeMemory(current);
                } else {
                    if(n>0){
                        DrawRectangleVGAbackground(0, 0, m-1, VRes-1,  blank) ;
                    } else {
                        DrawRectangleVGAbackground(HRes-m,0,HRes-1,VRes-1, blank) ;
                    }
                }
            } else 
#endif
#ifndef STM32F4version
			if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) {
                if(n>0){
                    if(blank<0)ReadBufferBuffFast8(HRes-m,0,HRes-1,VRes-1,current);
                    DoBlit( 0, 0, m, 0, HRes-m, VRes);
                    if(blank<0)DrawBufferBuffFast8(0,0,m-1,VRes-1,current);
                    else DrawRectangle(0, 0, m-1, VRes-1,  blank) ;
                } else {
                    if(blank<0)ReadBufferBuffFast8(0,0,m-1,VRes-1,current);
                    DoBlit( m, 0, 0, 0, HRes-m, VRes);
                    if(blank<0)DrawBufferBuffFast8(HRes-m,0,HRes-1,VRes-1, current);
                    else DrawRectangle(HRes-m,0,HRes-1,VRes-1, blank);
                }
                FreeMemory(current);
            } else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16){
                if(n>0){
                    if(blank<0)ReadBufferBuffFast(HRes-m,0,HRes-1,VRes-1,current);
                    DoBlit( 0, 0, m, 0, HRes-m, VRes);
                    if(blank<0)DrawBufferBuffFast(0,0,m-1,VRes-1,current);
                    else DrawRectangle(0, 0, m-1, VRes-1,  blank) ;
                } else {
                    if(blank<0)ReadBufferBuffFast(0,0,m-1,VRes-1,current);
                    DoBlit( m, 0, 0, 0, HRes-m, VRes);
                    if(blank<0)DrawBufferBuffFast(HRes-m,0,HRes-1,VRes-1, current);
                    else DrawRectangle(HRes-m,0,HRes-1,VRes-1, blank);
                }
                FreeMemory(current);
            } else {
#endif
                if(n>0){
                    if(blank<0)ReadBuffer(HRes-m,0,HRes-1,VRes-1,current);
                    DoBlit( 0, 0, m, 0, HRes-m, VRes);
                    if(blank<0)DrawBuffer(0,0,m-1,VRes-1,current);
                    else DrawRectangle(0, 0, m-1, VRes-1,  blank) ;
                } else {
                    if(blank<0)ReadBuffer(0,0,m-1,VRes-1,current);
                    DoBlit( m, 0, 0, 0, HRes-m, VRes);
                    if(blank<0)DrawBuffer(HRes-m,0,HRes-1,VRes-1, current);
                    else DrawRectangle(HRes-m,0,HRes-1,VRes-1, blank);
                }
                FreeMemory(current);
#ifndef STM32F4version
            }
#endif
            for(i=0; i< zeroLIFOpointer; i++){
#ifndef STM32F4version
            	if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
            	else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#if defined largechip
                else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
#endif
                else 
#endif
				BlitShow(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
             }
            for(i=0; i< LIFOpointer; i++){
                if(blitbuff[LIFO[i]].next_x != 10000){
                    blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                    blitbuff[LIFO[i]].next_x = 10000;
                }
                if(blitbuff[LIFO[i]].next_y != 10000){
                    blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                    blitbuff[LIFO[i]].next_y = 10000;
                }

#ifndef STM32F4version
                if (Option.DISPLAY_TYPE>=SSD1963_5_8BIT) BlitShowBuff8(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
                else if((Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER) || Option.DISPLAY_TYPE==SSD1963_4_16)BlitShowBuff(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#if defined largechip
                else if(Option.DISPLAY_TYPE==VGA)BlitShowVGA(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
#endif
                else 
#endif
				BlitShow(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
             }
            ProcessCollisions(0);
#ifndef STM32F4version
            if(Option.Refresh)Display_Refresh();
            if(Option.DISPLAY_TYPE==VGA)flipbuffer();
#endif
        }
    } else {
        getargs(&cmdline, 11,",");
        if(argc !=11) error("Syntax");
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        x2 = getinteger(argv[4]);
        y2 = getinteger(argv[6]);
        w = getinteger(argv[8]);
        h = getinteger(argv[10]);
        if(w < 1 || h < 1) return;
        if(x1 < 0) { x2 -= x1; w += x1; x1 = 0; }
        if(x2 < 0) { x1 -= x2; w += x2; x2 = 0; }
        if(y1 < 0) { y2 -= y1; h += y1; y1 = 0; }
        if(y2 < 0) { y1 -= y2; h += y2; y2 = 0; }
        if(x1 + w > HRes) w = HRes - x1;
        if(x2 + w > HRes) w = HRes - x2;
        if(y1 + h > VRes) h = VRes - y1;
        if(y2 + h > VRes) h = VRes - y2;
        if(w < 1 || h < 1 || x1 < 0 || x1 + w > HRes || x2 < 0 || x2 + w > HRes || y1 < 0 || y1 + h > VRes || y2 < 0 || y2 + h > VRes) return;
#if defined(largechip)
        if(Option.DISPLAY_TYPE == VGA){
            VGAbuffcopy();
            DoBlitVGA( x1, y1, x2, y2, w, h);
        } else
#endif

        DoBlit( x1, y1, x2, y2, w, h);
#ifndef STM32F4version
        if(Option.Refresh)Display_Refresh();
        if(Option.DISPLAY_TYPE==VGA)flipbuffer();
#endif
    }
}

