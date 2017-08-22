#include <MidWay.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>


int putsvc(mwsvcinfo *si) {
   FILE * fp;
   if (si->datalen  == 0) {      
      mwreturn ("error expeced 'key=value'", MWFAIL, 0, 0);
   }
   char * kvname = si->data;
   char * kvdata = index(si->data, '=');
   if (kvdata == NULL) {
      mwlog(MWLOG_DEBUG, "cant parse %s", si->data);
      mwreturn ("error expeced 'key=value'", MWFAIL, 0, 0);
   }
  
   mwlog(MWLOG_DEBUG, "PUT %s = %s", kvname, kvdata);

   *kvdata = '\0';
   kvdata++;
   int vallen = si->datalen - strlen(kvname) -1;
   fp = fopen(kvname, "w");
   fwrite(kvdata, 1, vallen, fp);
   fclose(fp);

   mwlog(MWLOG_DEBUG, "invalidate  %s ", kvname);
   int rc = mweventbcast("kvinvalidate", kvname, 0);

   mwlog(MWLOG_DEBUG, "good end");
   mwreturn ("OK", 0, MWSUCCESS, 0);
};

int getsvc(mwsvcinfo *si) {
   FILE * fp;
   int rc;
   if (si->datalen  == 0) {
      mwreply ("error expeced 'key'", 0, MWFAIL, 0, 0);
      return 0;
   }
   char * kvname = si->data;

   struct stat statbuf;
   rc = lstat(kvname,  &statbuf);
   if (rc != 0) {
      mwreply ("key not set ", 0, MWFAIL, 0, 0);
      return 0;
   }

   char * rpl = mwalloc(statbuf.st_size);
   fp = fopen(kvname, "r");
   if (fp == NULL)  {
      mwreply ("no key", 0, MWFAIL, 0, 0);
      return 0;
   }

   rc = fread(rpl, 1, statbuf.st_size, fp);
   mwreply (rpl, rc, MWSUCCESS, 0, 0);   
   
   fclose(fp);
   
   rc = mweventbcast("kvinvalidate", kvname, 0);

   return 0;
};


int delsvc(mwsvcinfo *si) {
   FILE * fp;
   int rc;
   if (si->datalen  == 0) {
      mwreply ("error expeced 'key'", 0, MWFAIL, 0, 0);
      return 0;
   }
   char * kvname = si->data;

   rc = unlink(kvname);
    if (rc != 0) {
       mwreply ("key not set ", 0, MWFAIL, 0, 0);
      return 0;
   }

   rc = mweventbcast("kvinvalidate", kvname, 0);

   return 0;
};


__attribute__((constructor))  void initializer(void)
{

   char * datadir = getenv("KVSTORE");
   if (datadir) {
      int rc = chdir (datadir);
      if (rc != 0) {
	 fprintf(stderr, "unable to chdir to datadir %s", datadir);
	 exit(-1);
      }
   }
   mwprovide("kvput", putsvc, 0);
   mwprovide("kvget", getsvc, 0);
   mwprovide("kvdel", delsvc, 0);
   //   mwprovide("kvcache", cachesvc, 0);
   //   mwprovide("kvstat", statsvc, 0);
   
};

__attribute__((destructor)) void finalizer(void)
{
 
};

