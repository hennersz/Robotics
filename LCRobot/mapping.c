#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/select.h>
#include "picomms.h"
#include "linkedList.h"
#include "turning.h"
#include "test.h"

double ratio;
static int socket1 = -1;
#define READBUFLEN 80

int connect_to_simulator() 
{
	printf("connecting to simulator...");
	struct sockaddr_in s_addr;
	if (socket1 != -1)
	{
    	close(socket1);
    	socket1 = -1;
  	}

  	if ((socket1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stderr, "Failed to create socket\n");
    	exit(1);
  	}
  	while (1) 
  	{
	    s_addr.sin_family = AF_INET;
	    
	    //You IP
	    s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	    s_addr.sin_port = htons(55440);

	    if (connect(socket1, (struct sockaddr *) &s_addr, sizeof(s_addr)) >= 0) 
	    {
		      /* connection succeeded */
		      printf("done\n");
		      sleep(1);
		      return socket1;
	    }
	    sleep(1);
	    printf(".");
	    fflush(stdout);
  	}
}

int send_msg1(char* msg, int len) 
{
	  if (write(socket1, msg, len) <= 0)
	  {
	    /* the write failed - likely the robot was switched off - attempt
		       to reconnect and reinitialize */
		    printf("send failed - reconnecting\n");
		    connect_to_simulator();
		    return 0;
	  } 
	  else 
	  {
	    	return 1;
	  }
}

/* strcpy isn't safe to use with overlapping data */
/* this one is safe, so long as you're copying to the left*/
void safestrcpy1(char *s1, char *s2, int len) 
{
	  int i;
	  assert(s1 < s2 || s1 > s2+len);
	  /* copy up to len bytes */
	  for(i = 0; i < len; i++) 
	  {
		    /* don't copy past the end of s2 */
		    if(*s2 == '\0')
		      break;
		    *s1++ = *s2++;
	  }
	  *s1 = '\0';
}

void add_to_buffer1(char *permbuf, unsigned int *permbuflen, char *tmpbuf) {
  char *end = permbuf + *permbuflen;
  int tmpbuflen = strlen(tmpbuf);
  assert(tmpbuflen < READBUFLEN);

  /* is there space?*/
  if (tmpbuflen + *permbuflen + 1 >= READBUFLEN) {
    /* we've no more space for the commands - looks like someone forgot some newlines! */
    /* clear the old stuff out of permbuf - it's no use to us */
    printf("cleaing old stuff\n");
    strcpy(permbuf, tmpbuf);
    return;
  }
  
  safestrcpy1(end, tmpbuf, tmpbuflen);
  *permbuflen += tmpbuflen;
}

/* extract a newline-terminated reply from a buffer */
const char* extract_reply1(char *buf) 
{
	  static char replybuf[READBUFLEN];
	  int len;
	  char *newline = strchr(buf, '\n');
	  
	  if (newline == NULL) 
	  {
		    /* we've not got a newline, so no finished command here */
		    return NULL;
	  }
	  *newline++ = '\0';
	  strcpy(replybuf, buf);
	  len = strlen(replybuf);
	  if (replybuf[len-1] == '\r')
	  {
	    	replybuf[len-1] = 0;
	    	len--;
	  }
	  if (replybuf[len-1] != '\n') 
	  {
	    	replybuf[len] = '\n';
	    	replybuf[len+1] = '\0';
	    	len++;
	  }
	  safestrcpy1(buf, newline, strlen(newline));
	  return replybuf;
}

const char* recv_msg1(char *buf, int bufsize) 
{
	  int val;
	  fd_set read_fdset;
	  fd_set except_fdset;
	  struct timeval tv;
	  char readbuf[bufsize];
	  unsigned int totalbytes;

	  /* Is there already a reply in the buffer from before? (can happen if
	     we get multiple lines in one read) */
	  const char *reply_msg = extract_reply1(buf);
	  if (reply_msg != NULL)
	  {
	    	return reply_msg;
	  }

	  while (1) {
		    tv.tv_sec = 1;
		    tv.tv_usec = 0;
		    FD_ZERO(&read_fdset);
		    FD_ZERO(&except_fdset);
		    FD_SET(socket1, &read_fdset);
		    FD_SET(socket1, &except_fdset);
		    if (select(socket1+1, &read_fdset, NULL, &except_fdset, &tv) == 0) 
		    {
		      /* we've waited 2 seconds and got no response - too long - conclude
			 the socket is dead */
		      printf("timed out waiting response\n");
		      //connect_to_robot();
		      //initialize_robot();
		      return 0;
		    }
		    if (FD_ISSET(socket1, &except_fdset))
		    {
			      connect_to_simulator();
			      return 0;
			}
		  
		    assert(FD_ISSET(socket1, &read_fdset));
		    val = read(socket1, readbuf, bufsize-1);
		    if (val > 0) 
		    {

		    } 
		    else 
		    {
			      /* the write failed - likely the robot was switched off - attempt
				 to reconnect and reinitialize */
			      connect_to_simulator();
			      buf[0]='\0';
			      return 0;
		    }
		    /* ensure readbuf is null terminated */
		    readbuf[val] = '\0';
		    totalbytes = strlen(buf);
		    add_to_buffer1(buf, &totalbytes, readbuf);
		    reply_msg = extract_reply1(buf);
		    if (reply_msg != NULL)
		      return reply_msg;
	  }
}

void recv_ack1() 
{
	  char  buf[READBUFLEN];
	  const char *reply;
	  memset(buf, 0, READBUFLEN);
	  reply = recv_msg1(buf, READBUFLEN);
	  while (reply && reply[0] != '.') 
	  {
	    	printf("Unexpected reply: >>%s<<\n", reply);
	    	reply = recv_msg1(buf, READBUFLEN);
	  }
}

void set_point1(int x, int y) 
{
	  char  buf[READBUFLEN];
	  sprintf(buf, "C POINT %d %d\n", x, y);
	  if (send_msg1(buf, strlen(buf))) 
	  {
	    recv_ack1();
	  }
}

void set_motors1(int speed_l, int speed_r)
{
	  char  buf[READBUFLEN];
	  sprintf(buf, "M LR %d %d\n", speed_l, speed_r);
	  if (send_msg1(buf, strlen(buf)))
	  {
	  	  recv_ack1();
	  }
} 

void set_origin1()
{
	  char  buf[READBUFLEN];
	  sprintf(buf, "C ORIGIN\n");
	  if (send_msg1(buf, strlen(buf)))
	  {
	    recv_ack1();
	  }
}

void set_origin2(int x, int y)
{
	  char  buf[READBUFLEN];
	  sprintf(buf, "C ORIGIN %d %d\n", x, y);
	  if (send_msg1(buf, strlen(buf)))
	  {
	    recv_ack1();
	  }
}

void initialiseMapping(Mapping *mapping)
{
	mapping->previousAngle = 0;
	mapping->previousLeft = 0;
	mapping->previousRight = 0;
	mapping->x = 0;
	mapping->y = 0;
}

void calculateRatio()
{
	double wheelCirc = WHEELDIAM * M_PI;
	double robotCirc = WIDTH * M_PI;
	ratio = 1/(robotCirc/wheelCirc);
}
/*
double findAngle(Mapping mapping)
{
	double angle;
	angle = (mapping->previousLeft - mapping->previousRight)/2.0 * ratio;
	return angle;
}
*/
double clicksToMM(int clicks)
{
	double wheelCirc = WHEELDIAM * M_PI;
	return clicks*wheelCirc/360.0;
}

double toRadians(double angle)
{
	return (double)(angle * (M_PI/180));
}

double toDegrees(double angle)
{
	//printf("offset = %f\n", angle*(180.0/M_PI));
	return angle*(180.0/M_PI);
}

void encoderChange(int* previousLeft, int* previousRight, int* deltaL, int* deltaR)
{
	int lEnc, rEnc;
	get_motor_encoders(&lEnc, &rEnc);
	*deltaL = lEnc - *previousLeft;
	*deltaR = rEnc - *previousRight;
	*previousLeft = lEnc;
	*previousRight = rEnc;
	
}

void updatePreviousEncoders(int* previousLeft, int* previousRight)
{
	int lEnc, rEnc;
	get_motor_encoders(&lEnc, &rEnc);
	*previousLeft = lEnc;
	*previousRight = rEnc;
	
}

void straightDistance(int distance, Mapping *m)
{
	
	double angle = m->previousAngle;
	distance = round(clicksToMM(distance));
	m->y += (distance * (cos(angle)));
	m->x += (distance * (sin(angle))); 
}

double angleChange(int deltaL, int deltaR)
{
	double dl = clicksToMM(deltaL);
	double dr = clicksToMM(deltaR);
	double angle = (dl-dr)/WIDTH;
	return angle;
}

void positionChange(Mapping *m, int deltaL, int deltaR)
{
	double currentAngle = angleChange(deltaL, deltaR); 
	double dL = clicksToMM(deltaL);
	double dR = clicksToMM(deltaR);
	double rL = dL/currentAngle;
	double rR = dR/currentAngle;
	double rM = (rL+rR)/2.0;

	if(m->previousAngle == 0)
	{
		m->x -= rM - rM * cos(currentAngle);
		m->y += rM * sin(currentAngle); 
	}
	else
	{
		m->x -= rM * cos(m->previousAngle + currentAngle) - rM * cos(m->previousAngle);
		m->y += rM * sin(m->previousAngle + currentAngle) - rM * sin(m->previousAngle);
	}
	m->previousAngle += currentAngle;
}

void distanceTravelled(Mapping *mapping)
{
	int deltaL, deltaR;
	encoderChange(&mapping->previousLeft, &mapping->previousRight, &deltaL, &deltaR);
	if(deltaL == deltaR)
	{
		straightDistance(deltaL, mapping);
	}
	else
	{
		positionChange(mapping, deltaL, deltaR);
	}
	//set_point1((mapping->x)/10, (mapping->y)/10);
	//set_point((mapping->x)/10, (mapping->y/10));
	//double distance = sqrt((mapping->x)*(mapping->x) + (mapping->y)*(mapping->y));
	//double angle = atan((mapping->x)/(mapping->y));
	//angle *= (180/M_PI);
	//printf("Distance = %f\tangle = %f\n", distance, angle);
}

int checkOrientation(Mapping *mapping)
{
	double angle = mapping->previousAngle;
	if(angle > 2*M_PI)
		angle -= 2*M_PI;
	else if(angle < 0)
		angle += 2*M_PI;
	angle = toDegrees(angle);
	
	if(angle > -10 && angle < 10)  //do we need -10?
		return 0;
	else if(angle > 80 && angle < 100)
		return 1;
	else if(angle > 170 && angle < 190)
		return 2;
	else
		return 3;
}

int addressToX(int address)
{
	return (address%4) * 600 + XOFFSET;
}

int addressToY(int address)
{
	return (address/4) * 600 + 300 + YOFFSET;
}