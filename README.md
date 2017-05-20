# C-Stabilizer

# Introduction

This experiment was performed to measure the maximum throughput at eunomia. In https://github.com/chathurilanchana/riak_kv/blob/Riak-Kv-Causal-dev (erlang version of Eunomia service) we couldnt experiment the max throughput due to inefficient data structures in erlang. Therefore, in order to measure max throughput we wrote the eunomia service in c++, modified clients to directly connect to eunomia bypasing the vnodes and measured the throughput.

# Usage

1) Build With Makefile
2) Run the Multiplexer server with the necessary parameters

Required Parameters
-----------------------------------------------------------------------------------------

1) arg1:Number of vnodes (simulated clients)
2) arg2:delete log threshold (for gathering stats (how often to print stats))
3) arg3:labelReceiverPort
4) arg4:deliveryLabelSize arg5:IgnoreMsgCount (To ignore initial messages (to avoid the influence of startup delays))
