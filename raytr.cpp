#include<SDL2/SDL.h>
#include<iostream>
#include<random>
#include<vector>
#include<cstdint>
#include<cstdlib>
std::mt19937 mt;
union Mask{
	uint32_t n;
	uint8_t c[4];
};
struct Vector{
	double x;
	double y;
	double z;
};
Vector operator+(Vector l,Vector r){
	return {l.x+r.x,l.y+r.y,l.z+r.z};
}
Vector operator-(Vector l,Vector r){
	return {l.x-r.x,l.y-r.y,l.z-r.z};
}
Vector operator*(Vector v,double a){
	return {v.x*a,v.y*a,v.z*a};
}
Vector operator-(Vector v){
	return Vector{0,0,0}-v;
}
double len(Vector v){
	return  sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}
Vector regu(Vector v){
	double l=len(v);
	if(l==0)
		return {0,0,0};
	v.x/=l;
	v.y/=l;
	v.z/=l;
	return v;
}
double dot(Vector l,Vector r){
	return l.x*r.x+l.y*r.y+l.z*r.z;
}
struct Color{
	double r;
	double g;
	double b;
};
Color operator*(Color l,double a){
	return {l.r*a,l.g*a,l.b*a};
}
Color operator*(Color l,Color r){
	return {l.r*r.r,l.g*r.g,l.b*r.b};
}
Uint32 color_hex(Color c){
	return (int)(c.r*0xff)+((int)(c.g*0xff)<<8)+((int)(c.b*0xff)<<16);
}
struct Object{
	virtual double collision(Vector,Vector)=0;
	virtual Color ray(Vector,Vector,int)=0;
};
std::vector<Object*> objects;
Color ray(Vector,Vector,int);
struct Plane:Object{
	Vector v;
	Vector v2;
	Vector v3;
	double c;
	Plane(Vector v,Vector v2,Vector v3,double c):v(v),v2(v2),v3(v3),c(c){}
	double collision(Vector p,Vector vv){
		return -(dot(p,v)-c)/dot(v,vv);
	}
	Color ray(Vector p,Vector vv,int n){
		if(n>10)
			return {0,0,0};
		double cc=dot(p,v)-c;
		double vc=dot(v,vv);
		Vector ph=p+vv*(-cc/vc);
		Vector vvv=vv-v*dot(v,vv)*2;
		if(((int)(dot(v2,ph-v*c)/70.0)+(int)(dot(v3,ph-v*c)/70.0))%2==0)
			return Color{0.5,0.5,1}*::ray(ph+vvv*0.1,vvv,n+1);
		else
			return Color{0.5,1,0.5}*::ray(ph+vvv*0.1,vvv,n+1);
	}
};
struct Sphere:Object{
	Vector p;
	double r;
	Sphere(Vector p,double r):p(p),r(r){}
	double collision(Vector pp,Vector vv){
		double t=dot(p-pp,vv);
		double u=len(pp+vv*t-p);
		if(u>=r)
			return -1;
		double v=sqrt(r*r-u*u);
		double a=t-v;
		if(t+v>0&&a<0)
			a=t+v;
		return a;
	}
	Color ray(Vector pp,Vector vv,int n){
		if(n>10)
			return {0,0,0};
		double t=dot(p-pp,vv);
		double u=len(pp+vv*t-p);
		double v=sqrt(r*r-u*u);
		double a=t-v;
		if(t+v>0&&a<0)
			a=t+v;
		Vector q=pp+vv*a;
		Vector vq=regu(q-p);
		Vector vvv=vv-vq*dot(vq,vv)*2;
		return Color{1,0.5,0.5}*::ray(q+vvv*0.1,vvv,n+1);
	}
};
Color ray(Vector p,Vector v,int n){
	double l_min=-1;
	Object *o=nullptr;
	for(auto object:objects){
		double l=object->collision(p,v);
		if(l>0)
			if(l_min==-1||l<l_min){
				l_min=l;
				o=object;
			}
	}
	if(o)
		return o->ray(p,v,n);
	return Color{1,1,1};
}
Uint32 f(int x,int y){
	double r=0,g=0,b=0;
	std::uniform_real_distribution<> rnd(0,1);
	for(int i=0;i<10;i++){
		Vector p={0,0,0};
		Vector v={(double)x-320.0,(479.0-(double)y)-240.0,300};
		v.x+=rnd(mt);
		v.y+=rnd(mt);
		v.z+=rnd(mt);
		v=regu(v);
		Color c=ray(p,v,0);
		r+=c.r;
		g+=c.g;
		b+=c.b;
	}
	return (0xff<<24)|color_hex({r/10,g/10,b/10});
}
int main(int argc,char* argv[]){
	Mask rmask={},gmask={},bmask={},amask={};
	rmask.c[0]=0xff;
	gmask.c[1]=0xff;
	bmask.c[2]=0xff;
	amask.c[3]=0xff;
	SDL_Event ev;
	if(SDL_Init(SDL_INIT_VIDEO)<0){
		std::cerr<<"SDL_Init: "<<SDL_GetError()<<std::endl;
		exit(1);
	}
	SDL_Window* window;
	window=SDL_CreateWindow("game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_SHOWN);
	if(window==NULL){
		std::cerr<<"SDL_CreateWindow: "<<SDL_GetError()<<std::endl;
		exit(2);
	}
	SDL_Renderer* renderer;
	renderer=SDL_CreateRenderer(window,-1,0);
	SDL_Surface *surface;
	surface=SDL_CreateRGBSurface(0,640,480,32,rmask.n,gmask.n,bmask.n,amask.n);
	uint32_t *pixels=(uint32_t*)surface->pixels;
	std::random_device rd;
	objects.push_back(new Plane{Vector{0,1,0},Vector{1,0,0},Vector{0,0,1},-30});
	objects.push_back(new Sphere{Vector{100,0,200},50});
	objects.push_back(new Sphere{Vector{-40,20,100},50});
	for(int i=0;i<surface->h;i++){
		for(int j=0;j<surface->w;j++){
			*pixels=f(j,i);
			pixels++;
		}
	}
	SDL_Texture *texture;
	texture=SDL_CreateTextureFromSurface(renderer,surface);
	SDL_SetRenderDrawColor(renderer,0,0,0,255);
	SDL_RenderClear(renderer);
	SDL_Rect src={},dst={};
	src.x=0;
	src.y=0;
	src.w=640;
	src.h=480;
	dst.x=0;
	dst.y=0;
	dst.w=640;
	dst.h=480;
	SDL_RenderCopy(renderer,texture,&src,&dst);
	SDL_RenderPresent(renderer);
	bool quitting=false;
	while(!quitting){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type==SDL_QUIT){
				quitting=true;
			}
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer,texture,&src,&dst);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
