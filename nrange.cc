#include "nrange.h"
#include "misc.h"
#include "log.h"

void c_nrange::insert(ulong n){
	t_rlist::iterator i=rlist.lower_bound(n),j;
	if (i!=rlist.end()){
		if ((*i).second<=n)
			return;//already in a range
		if ((*i).second==n+1){
			(*i).second=n;
			//					return;//1 below a range, don't return because we need to check for being 1 above another range
		}else
			i=rlist.end();//no good.
	}
	changed=1;//if we've gotten here, something will change.
	if (n>0){//don't wanna wrap around, since we are using unsigned
		j=rlist.find(n-1);
		if (j!=rlist.end()){
			if (i!=rlist.end())
				(*i).second=(*j).second;//1 above and 1 below two different ranges
			else
				rlist[n]=(*j).second;//only 1 above a range
			rlist.erase(j);
			return;
		}
	}
	if (i==rlist.end())
		rlist[n]=n;//need a new range
}
/*totally non working.  blah.  prolly would be easier to just start over.
   void c_nrange::insert(ulong l, ulong h){
	t_rlist::iterator i=rlist.lower_bound(h),j,g;
	int got1=0;
	if (i==rlist.end()){
		if (rlist.size())
			--i;
		else {
			pair<t_rlist::iterator, bool> pa=rlist.insert(t_rlist::value_type(h,l));//new range compelety covers this range
			got1=1;
			i=g=pa.first;
		}
	}
	while (i!=rlist.end()){
		j=i;
		--i;
		if (h > (*j).first){
			if (l <= (*j).first) {//range intersects (some or all) of this range
				if (l > (*j).second){//range intersects only the top part
					if (got1)
						(*g).second=(*j).second;//add the rest of this range onto the one we've already extended
					else{
						rlist[h]=(*j).second;//extend this range
					}
					rlist.erase(j);
					changed=1;
					return;//we're done.
				}
				else if (!got1){
					pair<t_rlist::iterator, bool> pa=rlist.insert(t_rlist::value_type(h,l));//new range compelety covers this range
					got1=1;
					g=pa.first;
				}
				rlist.erase(j);
				changed=1;
			}else{
				if (!got1)
					rlist[h]=l;//add the range if we haven't yet.
				return;//range is completely above this range, so we are done
			}
		}else if (!got1){
			if (h >= (*j).second) {//range intersects (some) of the lower of this range
				if (l > (*j).second) {
					return;//range is completly in the middle of a current range
				}
				(*j).second=l;
				changed=1;
				got1=1;
				g=j;
			}
			//					delete j;
		}

	}
}*/
void c_nrange::remove(ulong l, ulong h){
	if (debug){printf("remove (%lu-%lu) nrange...",l,h);fflush(stdout);}
	if (rlist.empty()){
		if (debug)printf("empty\n");
		return;
	}
	int ssize=0;
	if (debug)ssize=rlist.size();
	t_rlist::iterator i=rlist.lower_bound(h),j=rlist.end();
	if (i==rlist.end())
		--i;
	//while (i!=rlist.end()){
	while (j!=rlist.begin() && i!=rlist.end()){
		j=i;
		--i;
		if (h >= (*j).first){
			if (l <= (*j).first) {//remove range cuts off (some or all) of this range
				if (l > (*j).second)
					rlist[l-1]=(*j).second;//remove range cuts off only the top part
				rlist.erase(j);
				changed=1;
			}else{
				if (debug)printf("a(%i)\n",ssize-rlist.size());
				return;//remove range is completely above this range, so we are done
			}
		}else{
			if (h >= (*j).second) {//remove range cuts off (some) of the lower of this range
				if (l > (*j).second) {
					rlist[l-1]=(*j).second;//remove range cuts through the middle of a current range
				}
				(*j).second=h+1;
				changed=1;
			}
			//					delete j;
		}

	}
	if (debug)printf("b(%i)\n",ssize-rlist.size());
}
c_nrange::c_nrange(string fn){
	changed=0;
	file=fn;
	if (file.empty())
		return;
#ifdef HAVE_LIBZ
	c_file_gz f;
	file.append(".gz");
#else
	c_file_fd f;
#endif
	f.initrbuf(256);
	if (!f.open(file.c_str(),
#ifdef HAVE_LIBZ
				"r"
#else
				O_RDONLY
#endif
			   )){
		char *lp,*hp;
		while (f.bgets()>0){
			lp=f.rbufp;
			hp=strchr(lp,'-');
			if (hp)//note that we do no validity/overlap/etc checking here.  it is assumed the file was created correctly.
				rlist[atoul(hp+1)]=atoul(lp);
			else
				rlist[atoul(lp)]=atoul(lp);
		}
		f.close();
	}
}
c_nrange::~c_nrange(){
	if (file.empty())
		return;
#ifdef HAVE_LIBZ
	c_file_gz f;
#else
	c_file_fd f;
#endif
	if (changed && !f.open(file.c_str(),
#ifdef HAVE_LIBZ
				"w"
#else
				O_CREAT|O_WRONLY|O_TRUNC
#endif
			   )){
		if (debug){printf("saving nrange: %i contiguous ranges..",rlist.size());fflush(stdout);}
		print(&f);	
		if (debug) printf(" done.\n");
		f.close();
	}
}
