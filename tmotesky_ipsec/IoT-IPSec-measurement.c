/*
This file is based on Raza et al. (2011) - DOI 10.1109/DCOSS.2011.5982177
This file work in Contiki 2.7
 */

#include "contiki.h"
#include "uip.h"
#include "ipsec.h"
#include "net/uip-udp-packet.h"
#include "energest.h"

#include <stdio.h> /* For printf() */
#include <string.h>

#include "dev/sht11-sensor.h" /* used to obtain temperature and humidity */
#include "dev/light-sensor.h" /* used to obtain light */

PROCESS(iot_ipsec_process, "IoT6Sec IPSec Energy");
AUTOSTART_PROCESSES(&iot_ipsec_process);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

#define MOTE_PORT 1234
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

/* Counter */
int i;
i = 0;

/* structure to store connection parameters */
static struct uip_udp_conn *server_conn;

/* variable to store energest values for CPU */
static unsigned long cpu_time;

/* function to send data */
static void
tcpip_handler(void)
{
  char* dados = uip_appdata;

  /* Storing initial energest values */
  cpu_time = energest_type_time(ENERGEST_TYPE_CPU);

  if(uip_newdata()) {
    uip_len = 0;

    /* Activate sensors */
    SENSORS_ACTIVATE(light_sensor);
    SENSORS_ACTIVATE(sht11_sensor);

    /* Store in dados variable */
    sprintf(dados, "T:%d.U:%d.L:%d.\n", (((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10), (sht11_sensor.value(SHT11_SENSOR_HUMIDITY)), (10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7));

    /* Activate sensors */
    SENSORS_DEACTIVATE(light_sensor);
    SENSORS_DEACTIVATE(sht11_sensor);

    dados = dados+uip_ext_len;

    /* Send data */
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;
    uip_udp_packet_send(server_conn, dados, strlen(dados));
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
    server_conn->rport = 0;

    /* Print energest CPU energy consumption */
    printf("Energy consumption(%d): CPU: %lu\n",i,energest_type_time(ENERGEST_TYPE_CPU)-cpu_time);


  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(iot_ipsec_process, ev, data)
{
  PROCESS_BEGIN();

  /* new connection with remote host */
  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(MOTE_PORT));


  /* wait for incoming data */
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {

      i++;
      tcpip_handler();

    } // endif tcpip_event
  } // end while

  PROCESS_END();
}  //end process
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void dummy(void) {}
const struct uip_fallback_interface rpl_interface = {
    dummy, dummy
};
/*---------------------------------------------------------------------------*/
