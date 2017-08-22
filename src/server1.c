#include <MidWay.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char * subcommand(char * cmdstring) {
   char * rpl = mwalloc(1024);
   FILE * fp = popen(cmdstring, "r");
   fread (rpl, 1024, 1, fp);
   fclose(fp);
   return rpl;
};

int datesvc(mwsvcinfo *si) {
   time_t t = 0;
   if (si->datalen > 0 && sscanf(si->data, "%ld", &t) ==  1) {

   } else {
      time(&t);
   }
   char * s = ctime(&t);
   mwreply (s, strlen(s), MWSUCCESS, 0, 0);
   return 0;
};

int echosvc(mwsvcinfo *si) {
   mwreply (si->data, si->datalen, MWSUCCESS, 0, 0);
   return 0;
};

int eventsvc(mwsvcinfo *si) {
   if (si->datalen  == 0)
      mwreturn ("error expeced 'eventname data'", MWFAIL, 0, 0);
   char * evname = si->data;
   char * evdata = index(si->data, ' ');
   if (evdata == NULL)
      mwreturn ("error expeced 'eventname data'", MWFAIL, 0, 0);

   *evdata = '\0';
   evdata++;

   
   int rc = mweventbcast(evname, evdata, 0);

   mwreturn ("OK", 0, MWSUCCESS, 0);
};

int uptimesvc(mwsvcinfo *si) {
   char * rpl = subcommand("uptime");
   mwreturn (rpl, 0, MWSUCCESS, 0);
};

struct element {
   char * data;
   int datalen;
   struct element * next;
} * stacktop = NULL, * stackbottom = NULL;

int stacksvc(mwsvcinfo *si) {
   mwlog(MWLOG_DEBUG, "stack %s %d", si->service, strlen(si->service));
   if (strcmp(si->service, "push") == 0) {
      mwlog(MWLOG_DEBUG, "pushing");
      struct element * elm = malloc(sizeof(struct element));
      elm->next = stacktop;
      elm->datalen = si->datalen;
      // rather important. failure to copy into memory will cause buffer leak in SHM
      elm->data = malloc(elm->datalen);
      memcpy(elm->data, si->data, elm->datalen);
      stacktop = elm;
      if (stackbottom == NULL)
	 stackbottom = stacktop;
      mwlog(MWLOG_DEBUG, "pushed %d", elm->datalen);
      mwreply("", 0, MWSUCCESS, 0, 0);
      return 0;
   }
   if (strcmp(si->service, "pop") == 0) {
      mwlog(MWLOG_DEBUG, "poping");
      if (stacktop == NULL) {
	 mwreply("", 0, MWFAIL, 0, 0);
	 return 0;
      }
      struct element * elm = stacktop;
      mwlog(MWLOG_DEBUG, "pop %d", elm->datalen);
      int rc = mwreply (elm->data, elm->datalen, MWSUCCESS, 0, 0);
      stacktop = stacktop->next;
      if (stacktop == NULL) stackbottom = NULL;
      return 0;
   }
  
   if (strcmp(si->service, "peek") == 0) {
      mwlog(MWLOG_DEBUG, "peeking");
      if (stacktop == NULL) {
	 mwreply("", 0, MWFAIL, 0, 0);
	 return 0;
      }
      struct element * elm = stacktop;
      mwlog(MWLOG_DEBUG, "peep %d", elm->datalen);
      int rc = mwreply (elm->data, elm->datalen, MWSUCCESS, 0, 0);
      return 0;
   }
   if (strcmp(si->service, "size") == 0) {
      mwlog(MWLOG_DEBUG, "size");
      int count = 0;
      struct element * elm = stacktop;
      while(elm != NULL) {
	 count ++;
	 elm = elm->next;
      }
      char * rpl = mwalloc(60);
      sprintf(rpl, "stack.len:=%d", count);
      mwreply(rpl, 0, MWSUCCESS, 0, 0);
      return 0;
   }
   
   mwreply("unknown command", 0, MWFAIL, 0, 0);
   return 0;   
};

int multisvc(mwsvcinfo *si) {
   char buf[64];
   int l;
   for (int i = 0; i < 10 ; i++) {
      l = sprintf(buf, "reply %d to call %s\n", i+1, si->service);

      mwlog(MWLOG_DEBUG, "reply");
      mwreply (buf, l, (i != 9 ? MWMORE : MWSUCCESS) , i, 0);
      mwlog(MWLOG_DEBUG, "repled");
   }
   return 0;
};


__attribute__((constructor))  void init(void)
{
   printf("in const\n");
   mwprovide("date", datesvc, 0);
   mwprovide("echo", echosvc, 0);
   mwprovide("event", eventsvc, 0);
   mwprovide("pop", stacksvc, 0);
   mwprovide("push", stacksvc, 0);
   mwprovide("peek", stacksvc, 0);
   mwprovide("size", stacksvc, 0);      
   mwprovide("uptime", uptimesvc, 0);
   mwprovide("multi", multisvc, 0);   
};

__attribute__((destructor)) void finalizer(void)
{
   printf("in dest\n");
   mwunprovide("date");
   mwunprovide("echo");
   mwunprovide("event");
   mwunprovide("pop");
   mwunprovide("push");
   mwunprovide("uptime");
   mwunprovide("multi");
};

