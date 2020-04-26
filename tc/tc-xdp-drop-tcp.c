#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/pkt_cls.h>

#include "bpf_endian.h"
#include "bpf_helpers.h"

#define SEC(NAME) __attribute__((section(NAME), used))

SEC("xdp")
int xdp_drop_tcp(struct struct xdp_md *ctx)
{

  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;

  if isTCP(data, data_end)
    return XDP_DROP;

  return XDP_PASS;
}

SEC("tc")
int tc_drop_tcp(struct __sk_buff *skb)
{

  void *data = (void *)(long)skb->data;
  void *data_end = (void *)(long)skb->data_end;

  if isTCP(data, data_end) 
    return TC_ACT_SHOT;

  return TC_ACT_OK;
}

/*
  check whether the packet is of TCP protocol
*/
bool isTCP(void *data_begin, void *data_end)
{
  struct ethhdr *eth = data_begin;

  // Check packet's size
  // the pointer arithmetic is based on the size of data type, current_address plus int(1) means:
  // new_address= current_address + size_of(data type)
  if (eth + 1 > data_end) //
    return false;

  // Check if Ethernet frame has IP packet
  if (eth->h_proto == bpf_htons(ETH_P_IP))
  {
    struct iphdr *iph = (struct iphdr *)(eth + 1); // or (struct iphdr *)( ((void*)eth) + ETH_HLEN );
    if (iph + 1 > data_end)
      return false;

    // Check if IP packet contains a TCP segment
    if (iph->protocol == IPPROTO_TCP)
      return true;
  }

  return false;
}

char _license[] SEC("license") = "GPL";


// int ipsize = 0;
//   struct ethhdr *eth = data;
//   ipsize = sizeof(*eth);

//   struct iphdr *ip = data + ipsize;
//   ipsize += sizeof(struct iphdr);
//   if (data + ipsize > data_end)
//   {
//     return TC_ACT_OK;
//   }

//   if (ip->protocol == IPPROTO_TCP)
//   {
//     return TC_ACT_SHOT;
//   }

//   return TC_ACT_OK;