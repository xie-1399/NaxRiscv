
#include <verilated.h>
#include "verilated_fst_c.h"
#include "VNaxRiscv.h"
#include "VNaxRiscv_NaxRiscv.h"

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <cstring>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <iomanip>
#include <queue>
#include <time.h>
#include <map>
#include <filesystem>
#include <chrono>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "disasm.h"
#include "type.h"


#include <execinfo.h>
#include <signal.h>

void handler_crash(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);
  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO); //函数调用路径
  exit(1);
}


#define CSR_UCYCLE 0xC00
#define CSR_UCYCLEH 0xC80

using namespace std;

class successException : public std::exception { };
#define failure() throw std::exception();
#define success() throw successException();

#define VL_RANDOM_I_WIDTH(w) (VL_RANDOM_I() & (1 << w)-1)

class SimElement{
public:
    bool withoutReset = false;
    virtual ~SimElement(){}
    virtual void onReset(){}
    virtual void postReset(){}
    virtual void preCycle(){}
    virtual void postCycle(){}
};

class SocElement{
public:
    u64 mappingStart, mappingEnd;

    virtual ~SocElement(){}
    virtual void onReset(){}
    virtual void postReset(){}

    virtual int write(u64 address, uint32_t length, uint8_t *data) = 0;
    virtual int read(u64 address, uint32_t length, uint8_t *data) = 0;
};

#include "memory.h"
#include "elf.h"
#include "nax.h"
#include "jtag.h"
#include "simple_block_device.h"
#include "framebuffer.h"
#define u64 uint64_t
#define u8 uint8_t

#ifndef FLOAT_WRITE_COUNT
#define FLOAT_WRITE_COUNT 0
#endif

#include <stdio.h>
#include <getopt.h>

#define RvAddress u32

#if XLEN == 32
#define RvData u32
#else
#define RvData u64
#endif

#ifdef RVD
#define RvFloat u64
#else
#define RvFloat u32
#endif


#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define CALL(x,y,z) MAP_ ## x(y,z)
#define MAP_1(prefix, postfix) prefix ## 0 ## postfix
#define MAP_2(prefix, postfix) MAP_1(prefix, postfix), prefix ## 1 ## postfix
#define MAP_3(prefix, postfix) MAP_2(prefix, postfix), prefix ## 2 ## postfix
#define MAP_4(prefix, postfix) MAP_3(prefix, postfix), prefix ## 3 ## postfix
#define MAP_5(prefix, postfix) MAP_4(prefix, postfix), prefix ## 4 ## postfix
#define MAP_6(prefix, postfix) MAP_5(prefix, postfix), prefix ## 5 ## postfix
#define MAP_7(prefix, postfix) MAP_6(prefix, postfix), prefix ## 6 ## postfix
#define MAP_8(prefix, postfix) MAP_7(prefix, postfix), prefix ## 7 ## postfix
#define MAP_9(prefix, postfix) MAP_8(prefix, postfix), prefix ## 8 ## postfix
#define MAP_10(prefix, postfix) MAP_9(prefix, postfix), prefix ## 9 ## postfix
#define MAP_11(prefix, postfix) MAP_10(prefix, postfix), prefix ## 10 ## postfix
#define MAP(type, name, prefix, count, postfix) type name[] = {CALL(count, prefix, postfix)};
#define MAP_INIT(prefix, count, postfix) CALL(count, prefix, postfix)

//simulation time记录，按照时钟周期进行递增
vluint64_t main_time = 0;

#define SIM_MASTER_PORT 18654
#define SA struct sockaddr



#define TEXTIFY(A) #A
void breakMe(){
    int a = 0;
}

#if XLEN == 32
#define assertEq(message, x,ref) if((RvData)(x) != (RvData)(ref)) {\
	printf("\n*** %s DUT=%x REF=%x ***\n\n",message,(RvData)x,(RvData)ref);\
	breakMe();\
	failure();\
}
#else
#define assertEq(message, x,ref) if((RvData)(x) != (RvData)(ref)) {\
	printf("\n*** %s DUT=%lx REF=%lx ***\n\n",message,(RvData)x,(RvData)ref);\
	breakMe();\
	failure();\
}
#endif

#define assertTrue(message, x) if(!(x)) {\
    printf("\n*** %s ***\n\n",message);\
    breakMe();\
    failure();\
}

#include <sys/stat.h>

//Thanks https://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
int mkpath(std::string s,mode_t mode)
{
    size_t pos=0;
    std::string dir;
    int mdret;

    if(s[s.size()-1]!='/'){
        // force trailing / so we can handle everything in loop
        s+='/';
    }

    while((pos=s.find_first_of('/',pos))!=std::string::npos){
        dir=s.substr(0,pos++);
        if(dir.size()==0) continue; // if leading / first time is 0 length
        if((mdret=mkdir(dir.c_str(),mode)) && errno!=EEXIST){
            return mdret;
        }
    }
    return mdret;
}


#define CAUSE_MACHINE_SOFTWARE 3
#define CAUSE_MACHINE_TIMER 7
#define CAUSE_MACHINE_EXTERNAL 11
#define CAUSE_SUPERVISOR_EXTERNAL 9


#define MIE_MTIE (1 << CAUSE_MACHINE_TIMER)
#define MIE_MEIE (1 << CAUSE_MACHINE_EXTERNAL)
#define MIE_MSIE (1 << CAUSE_MACHINE_SOFTWARE)
#define MIE_SEIE (1 << CAUSE_SUPERVISOR_EXTERNAL)

#include "encoding.h"
#define MIP 0x344
#define SIP 0x144
#define UIP  0x44


#define BASE 0x10000000
#define PUTC BASE
#define PUT_HEX (BASE + 0x8)
#define CLINT_BASE (BASE + 0x10000)
#define CLINT_TIME (CLINT_BASE + 0x0BFF8)
#define MACHINE_EXTERNAL_INTERRUPT_CTRL (BASE+0x10)
#define SUPERVISOR_EXTERNAL_INTERRUPT_CTRL (BASE + 0x18)
#define GETC (BASE + 0x40)
#define STATS_CAPTURE_ENABLE (BASE + 0x50)
#define PUT_DEC (BASE + 0x60)
#define INCR_COUNTER (BASE + 0x70)
#define FAILURE_ADDRESS (BASE + 0x80)

#define LITE_UART (BASE + 0x8000)
#define LITE_UART_OFF_RXTX  	 (LITE_UART+0x00)
#define LITE_UART_OFF_TXFULL	 (LITE_UART+0x04)
#define LITE_UART_OFF_RXEMPTY	 (LITE_UART+0x08)
#define LITE_UART_OFF_EV_STATUS	 (LITE_UART+0x0c)
#define LITE_UART_OFF_EV_PENDING (LITE_UART+0x10)
#define LITE_UART_OFF_EV_ENABLE	 (LITE_UART+0x14)

/* events */
#define LITE_UART_EV_TX		0x1
#define LITE_UART_EV_RX		0x2


#define MM_FAULT_ADDRESS 0x00001230
#define IO_FAULT_ADDRESS 0x1FFFFFF0


#define CLINT_CMP_ADDR (CLINT_BASE + 0x4000)

bool traceWave = false;
bool trace_ref = false;
vluint64_t timeout = -1;
string simName = "???";
string outputDir = "output";
double progressPeriod = 0.0;
bool statsPrint = false;
bool statsPrintHist = false;
bool traceGem5 = false;
bool spike_debug = false;
bool spike_enabled = true;
bool timeout_enabled = true;
bool simMaster = false;
bool simSlave = false;
bool noStdIn = false;
bool putcFlush = true;
bool putcTimestamp = false;
bool passFailWritten = false;
bool putcNewLine = true;


class TestSchedule{
public:
    virtual void activate() = 0;
};


queue <TestSchedule*> testScheduleQueue;

void testScheduleQueueNext(){
    if(testScheduleQueue.empty()) return;
    auto e = testScheduleQueue.front();
    testScheduleQueue.pop();
    e->activate();
}



inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void simMasterGetC(char c);

bool stdinNonEmpty(){
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}



class Soc : public SimElement{
public:
    Memory memory;
    VNaxRiscv* nax;
    u64 clintCmp = 0;
    queue <char> customCin;
    string putcHistory = "ls";
    string *putcTarget = NULL;
    RvData incrValue = 0;

    vector<SocElement*> socElements;
    vector<function<void(u32, u32, u8*)>> snoopWrites;

    Soc(VNaxRiscv* nax){
        this->nax = nax;
    }

    SocElement * findElement(u64 address){
        for(auto e : socElements){
            if(address >= e->mappingStart && address <= e->mappingEnd) return e;
        }
        return NULL;
    }

    virtual int memoryRead(uint32_t address,uint32_t length, uint8_t *data){
        if(address < 0x10000000) return 1;
        memory.read(address, length, data);
        return 0;
    }

    virtual int memoryWrite(uint32_t address,uint32_t length, uint8_t *data){
        if(address < 0x10000000) return 1;
        memory.write(address, length, data);
        for(auto e : snoopWrites) e(address, length, data);
        return 0;
    }

    virtual int peripheralWrite(u64 address, uint32_t length, uint8_t *data){
//        if((address & 0xFFFFFFFFF0000000) != 0x10000000) return memoryWrite(address,length,data);
        switch(address){
            /*
             * PUTC : 定义输出字符的起始地址，其实就是根据地址进行输出
             */
        case LITE_UART_OFF_RXTX:
        case PUTC: {
            if(putcNewLine){
                putcNewLine = false;
                if(putcTimestamp){
                    printf("[SIM %09ld] ", main_time);
                }
            }
            if(*data == '\n'){
                putcNewLine = true;
            }
            printf("%c", *data); if(putcFlush) fflush(stdout);
            putcHistory += (char)(*data);
            if(putcTarget){
                if(ends_with(putcHistory, *putcTarget)){
                    putcTarget = NULL;
                    testScheduleQueueNext();
                    putcHistory = "";
                }
            }
        }break;
        case PUT_HEX: printf("%x", *((u32*)data)); if(putcFlush) fflush(stdout); break;
        case PUT_DEC: printf("%d", *((u32*)data)); if(putcFlush) fflush(stdout); break;
        case MACHINE_EXTERNAL_INTERRUPT_CTRL: nax->PrivilegedPlugin_io_int_machine_external = *data & 1;  break;
        #if SUPERVISOR == 1
        case SUPERVISOR_EXTERNAL_INTERRUPT_CTRL: nax->PrivilegedPlugin_io_int_supervisor_external = *data & 1;  break;
        #endif
        case CLINT_BASE: nax->PrivilegedPlugin_io_int_machine_software = *data & 1;  break;
        case CLINT_CMP_ADDR: memcpy(&clintCmp, data, length); /*printf("CMPA=%lx\n", clintCmp);*/ break;
        case CLINT_CMP_ADDR+4: memcpy(((char*)&clintCmp)+4, data, length); /*printf("CMPB=%lx\n", clintCmp);*/  break;
        case INCR_COUNTER: incrValue += *((RvData*) data);break;
        case FAILURE_ADDRESS: printf("software asked failure\n"); failure(); break;
        case LITE_UART_OFF_EV_PENDING: break;
        case LITE_UART_OFF_EV_ENABLE: break;
//        case STATS_CAPTURE_ENABLE: whitebox->statsCaptureEnable = *data & 1; break;
        default: {
            auto e = findElement(address);
            if(e) return e->write(address-e->mappingStart, length, data);
            return 1;
        }break;
        }
        return 0;
    }

    virtual int peripheralRead(u64 address, uint32_t length, uint8_t *data){
//        if((address & 0xFFFFFFFFF0000000) != 0x10000000) return memoryRead(address,length,data);
        switch(address){
        case LITE_UART_OFF_RXTX:
        case GETC:{
            if(!simSlave && !noStdIn && stdinNonEmpty()){
                char c;
                auto dummy = read(0, &c, 1);
                memset(data, 0, length);
                *data = c;
                if(simMaster){
                    simMasterGetC(c);
                }
                /*
                 * 使用队列保存输入的数据
                 */
            } else if(!customCin.empty()){
                memset(data, 0, length);
                *data = customCin.front();
                customCin.pop();
            } else {
                memset(data, 0xFF, length);
            }
        } break;
        case CLINT_TIME:{
            u64 time = main_time/2;
            memcpy(data, &time, length);
        } break;
        case CLINT_TIME+4:{
            u64 time = (main_time/2) >> 32;
            memcpy(data, &time, length);
        } break;
        case CLINT_CMP_ADDR:   memcpy(data, &clintCmp, length); break;
        case CLINT_CMP_ADDR+4: memcpy(data, ((char*)&clintCmp) + 4, length); break;
        case INCR_COUNTER:     memcpy(data, ((char*)&incrValue), length); break;
        case LITE_UART_OFF_TXFULL:  { u64 value = VL_RANDOM_I_WIDTH(1); memcpy(data, &value, length); }break;
        case LITE_UART_OFF_RXEMPTY: { u64 value = !(!simSlave && !noStdIn && stdinNonEmpty()) && customCin.empty(); memcpy(data, &value, length); }break;
        default: {
            auto e = findElement(address);
            if(e) return e->read(address-e->mappingStart, length, data);
            return 1;
        }break;
        }
        return 0;
    }


    virtual void onReset(){
        nax->PrivilegedPlugin_io_int_machine_external = 0;
        nax->PrivilegedPlugin_io_int_machine_timer = 0;
        nax->PrivilegedPlugin_io_int_machine_software = 0;
        nax->PrivilegedPlugin_io_int_supervisor_external = 0;
        for(auto e : socElements) e->onReset();
    }

    virtual void postReset(){
        for(auto e : socElements) e->postReset();
    }
    virtual void preCycle(){

    }
    virtual void postCycle(){
        nax->PrivilegedPlugin_io_int_machine_timer = clintCmp < (main_time/2);
    }
};


Soc *soc;

class WaitPutc : public TestSchedule{
public:
    WaitPutc(string putc) : putc(putc){}
    string putc;
    void activate() {
        soc->putcTarget = &putc;
    }
};


class DoSuccess: public TestSchedule{
public:
    void activate() {
        success();
    }
};



class DoGetc : public TestSchedule{
public:
    string getc;
    DoGetc(string getc) : getc(getc){}
    void activate() {
        for(char e : getc){
            soc->customCin.push(e);
        }
        soc->customCin.push('\n');
        testScheduleQueueNext();
    }
};


class WithMemoryLatency{
public:
    virtual void setLatency(int cycles) = 0;
};

#define FETCH_MEM_DATA_BYTES (FETCH_MEM_DATA_BITS/8)

class FetchCached : public SimElement, public WithMemoryLatency{
public:
	bool error_next = false;
	u64 pendingCount = 0;
	u64 address;
	u64 time;
	bool stall;

    VNaxRiscv* nax;
    Soc *soc;

    u32 readyTrigger = 100;
    u32 latency = 2;
    void setLatency(int cycles){
        latency = cycles*2;
    }
    void setBandwidth(float ratio){
        readyTrigger = 128*ratio;
    }

	FetchCached(VNaxRiscv* nax, Soc *soc, bool stall){
		this->nax = nax;
		this->soc = soc;
		this->stall = stall;
	}

    //before Reset
	virtual void onReset(){
		nax->FetchCachePlugin_mem_cmd_ready = 1;
		nax->FetchCachePlugin_mem_rsp_valid = 0;
	}

	virtual void preCycle(){
#       //when cmd is coming(valid equals 1)
		if (nax->FetchCachePlugin_mem_cmd_valid && nax->FetchCachePlugin_mem_cmd_ready && pendingCount == 0) {
			assertEq("FETCH MISSALIGNED", nax->FetchCachePlugin_mem_cmd_payload_address & (FETCH_MEM_DATA_BYTES-1),0);
			pendingCount = FETCH_LINE_BYTES;
			address = nax->FetchCachePlugin_mem_cmd_payload_address;
			time = main_time + latency;
		}
	}

	virtual void postCycle(){
        //after dealing the cmd
		nax->FetchCachePlugin_mem_rsp_valid = 0;
        //有请求且且没有阻塞时
		if(pendingCount != 0 && (!stall || VL_RANDOM_I_WIDTH(7) < readyTrigger && time <= main_time)){
			nax->FetchCachePlugin_mem_rsp_payload_error = soc->memoryRead(address, FETCH_MEM_DATA_BYTES, (u8*)&nax->FetchCachePlugin_mem_rsp_payload_data);
			pendingCount-=FETCH_MEM_DATA_BYTES;
            //address FETCH_MEM_DATA_BYTES (FETCH_MEM_DATA_BITS/8) (No Branch?)
			address = address + FETCH_MEM_DATA_BYTES;
			nax->FetchCachePlugin_mem_rsp_valid = 1;
		}
		if(stall) nax->FetchCachePlugin_mem_cmd_ready = VL_RANDOM_I_WIDTH(7) < readyTrigger && pendingCount == 0;
	}
};


#define DATA_MEM_DATA_BYTES (DATA_MEM_DATA_BITS/8)

class DataCachedReadChannel{
public:
    u64 beats;
    u64 address;
    int id;

    u64 time;
};

class DataCachedWriteChannel{
public:
    u64 bytes;
    u64 address;
    int id;
    char buffer[DATA_LINE_BYTES];
};

class DataCachedWriteRspChannel{
public:
    bool valid;
    u64 bytes;
    u64 address;
    char data[DATA_LINE_BYTES];

    u64 time;
};


class DataCached : public SimElement, public WithMemoryLatency{
public:
    vector<DataCachedReadChannel> readChannels;
    DataCachedWriteChannel writeCmdChannel;
    vector<DataCachedWriteRspChannel> writeRspChannels;

    bool stall;

    VNaxRiscv* nax;
    Soc *soc;
    DataCachedReadChannel *chLock = NULL;

    u32 readyTrigger = 100;
    u32 latency = 2;
    void setLatency(int cycles){
        latency = cycles*2;
    }
    void setBandwidth(float ratio){
        readyTrigger = 128*ratio;
    }

    DataCached(VNaxRiscv* nax, Soc *soc, bool stall){
        this->nax = nax;
        this->soc = soc;
        this->stall = stall;
        for(int i = 0;i < DATA_CACHE_REFILL_COUNT;i++) {
            DataCachedReadChannel ch;
            ch.beats = 0;
            ch.id = i;
            readChannels.push_back(ch);
        }
        writeCmdChannel.bytes = 0;

        for(int i = 0;i < DATA_CACHE_WRITEBACK_COUNT;i++) {
            DataCachedWriteRspChannel ch;
            ch.valid = false;
            writeRspChannels.push_back(ch);
        }
    }


    virtual void onReset(){
        nax->DataCachePlugin_mem_read_cmd_ready = 1;
        nax->DataCachePlugin_mem_read_rsp_valid = 0;
        nax->DataCachePlugin_mem_write_cmd_ready = 1;
        nax->DataCachePlugin_mem_write_rsp_valid = 0;
    }

    virtual void preCycle(){
        if (nax->DataCachePlugin_mem_read_cmd_valid && nax->DataCachePlugin_mem_read_cmd_ready) {
#if DATA_CACHE_REFILL_COUNT == 1
            int id = 0;
#else
            int id = nax->DataCachePlugin_mem_read_cmd_payload_id;
#endif
            assertEq("CHANNEL BUSY", readChannels[id].beats, 0);
            readChannels[id].beats   = DATA_LINE_BYTES/DATA_MEM_DATA_BYTES;
            readChannels[id].address = nax->DataCachePlugin_mem_read_cmd_payload_address;
            readChannels[id].time    = main_time + latency;
        }

        if (nax->DataCachePlugin_mem_write_cmd_valid && nax->DataCachePlugin_mem_write_cmd_ready) {
#if DATA_CACHE_WRITEBACK_COUNT == 1
            int id = 0;
#else
            int id = nax->DataCachePlugin_mem_write_cmd_payload_fragment_id;
#endif
            if(!writeCmdChannel.bytes){
                writeCmdChannel.id = id;
                writeCmdChannel.address = nax->DataCachePlugin_mem_write_cmd_payload_fragment_address;
            }
            assert(id == writeCmdChannel.id);
            assert(writeCmdChannel.address == nax->DataCachePlugin_mem_write_cmd_payload_fragment_address);

            memcpy(writeCmdChannel.buffer + writeCmdChannel.bytes, &nax->DataCachePlugin_mem_write_cmd_payload_fragment_data, DATA_MEM_DATA_BYTES);
            //Why not writecmd latency
            writeCmdChannel.bytes += DATA_MEM_DATA_BYTES;
            //when write cmd is coming
            if(writeCmdChannel.bytes == DATA_LINE_BYTES){
                writeRspChannels[id].address = writeCmdChannel.address;
                writeRspChannels[id].bytes = writeCmdChannel.bytes;
                writeRspChannels[id].time = main_time + latency;
                writeRspChannels[id].valid = true;
                memcpy(writeRspChannels[id].data, writeCmdChannel.buffer, writeCmdChannel.bytes);
                writeCmdChannel.bytes = 0;
            }
        }

    }

    virtual void postCycle(){
        // Generate read responses
        nax->DataCachePlugin_mem_read_rsp_valid = 0;
        if(!stall || VL_RANDOM_I_WIDTH(7) < readyTrigger){
            if(chLock == NULL){
                int id = VL_RANDOM_I_WIDTH(7) % DATA_CACHE_REFILL_COUNT;
                for(int i = 0;i < DATA_CACHE_REFILL_COUNT; i++){
                    if(readChannels[id].beats != 0 && readChannels[id].time <= main_time){
                        chLock = &readChannels[id];
                        break;
                    }
                    id = (id + 1) % DATA_CACHE_REFILL_COUNT;
                }
            }

            if(chLock != NULL){
                nax->DataCachePlugin_mem_read_rsp_payload_error = soc->memoryRead(chLock->address, DATA_MEM_DATA_BYTES, (u8*)&nax->DataCachePlugin_mem_read_rsp_payload_data);
                nax->DataCachePlugin_mem_read_rsp_valid = 1;
#if DATA_CACHE_REFILL_COUNT != 0
                nax->DataCachePlugin_mem_read_rsp_payload_id = chLock->id;
#endif
                chLock->address = chLock->address + DATA_MEM_DATA_BYTES;
                chLock->beats -= 1;
                if(chLock->beats == 0){
                    chLock = NULL;
                }
            }
        }
        if(stall) nax->DataCachePlugin_mem_read_cmd_ready = VL_RANDOM_I_WIDTH(7) < readyTrigger;

        // Generate write responses
        nax->DataCachePlugin_mem_write_rsp_valid = 0;
        if(!stall || VL_RANDOM_I_WIDTH(7) < readyTrigger){
            DataCachedWriteRspChannel *ch = NULL;
            int id = VL_RANDOM_I_WIDTH(7) % DATA_CACHE_WRITEBACK_COUNT;
            for(int i = 0;i < DATA_CACHE_WRITEBACK_COUNT; i++){
                if(writeRspChannels[id].valid != 0 && writeRspChannels[id].time <= main_time){
                    ch = &writeRspChannels[id];
                    break;
                }
                id = (id + 1) % DATA_CACHE_REFILL_COUNT;
            }

            if(ch){
                ch->valid = false;
                nax->DataCachePlugin_mem_write_rsp_payload_error = soc->memoryWrite(ch->address, DATA_LINE_BYTES, (u8*)ch->data);
                nax->DataCachePlugin_mem_write_rsp_valid = 1;
#if DATA_CACHE_WRITEBACK_COUNT > 1
                nax->DataCachePlugin_mem_write_rsp_payload_id = id;
#endif
            }
        }
        if(stall) nax->DataCachePlugin_mem_write_cmd_ready = VL_RANDOM_I_WIDTH(7) < readyTrigger;
    }
};
//TODO randomize buses when not valid ^




class IoAccess{
public:
    u64 addr;
    u64 len;
    u8 data[8];
    bool write;
    bool error;
};

class LsuPeripheral: public SimElement{
public:
    bool valid;
    bool write;
    u64 address;
    u64 data;
    u64 mask;
    u64 bytes;

    bool stall;

    VNaxRiscv* nax;
    Soc *soc;
    queue<IoAccess> *mmioDut;
    DataCachedReadChannel *chLock = NULL;

    LsuPeripheral(VNaxRiscv* nax, Soc *soc, queue<IoAccess> *mmioDut, bool stall){
        this->nax = nax;
        this->soc = soc;
        this->stall = stall;
        this->mmioDut = mmioDut;
        valid = false;
    }

    virtual void onReset(){
        nax->LsuPlugin_peripheralBus_cmd_ready = 0;
        nax->LsuPlugin_peripheralBus_rsp_valid = 0;
    }

    virtual void preCycle(){
        if (nax->LsuPlugin_peripheralBus_cmd_valid && nax->LsuPlugin_peripheralBus_cmd_ready) {
            assert(!valid);
            address = nax->LsuPlugin_peripheralBus_cmd_payload_address;
            data = nax->LsuPlugin_peripheralBus_cmd_payload_data;
            mask = nax->LsuPlugin_peripheralBus_cmd_payload_mask;
            bytes = 1 << nax->LsuPlugin_peripheralBus_cmd_payload_size;
            write = nax->LsuPlugin_peripheralBus_cmd_payload_write;
            valid = true;
        }
    }

    virtual void postCycle(){
        // Generate read responses
        nax->LsuPlugin_peripheralBus_rsp_valid = 0;
        if(valid && (!stall || VL_RANDOM_I_WIDTH(7) < 100)){
            nax->LsuPlugin_peripheralBus_rsp_valid = 1;
            u64 offset = address & (LSU_PERIPHERAL_WIDTH/8-1);
            u8 *ptr = ((u8*) &data) + offset;
            assertTrue("bad io length\n", offset + bytes <= LSU_PERIPHERAL_WIDTH/8);

            if(write){
                nax->LsuPlugin_peripheralBus_rsp_payload_error = soc->peripheralWrite(address, bytes, ptr);
            } else {
                nax->LsuPlugin_peripheralBus_rsp_payload_error = soc->peripheralRead(address, bytes, ptr);
                memcpy(((u8*) &nax->LsuPlugin_peripheralBus_rsp_payload_data) + offset, ptr, bytes);
            }

            IoAccess access;
            access.addr = address;
            access.len = bytes;
            access.write = write;
            access.error = nax->LsuPlugin_peripheralBus_rsp_payload_error;
            memcpy(((u8*) &access.data), ptr, bytes);
            mmioDut->push(access);
            valid = false;
        }
        if(stall) nax->LsuPlugin_peripheralBus_cmd_ready = VL_RANDOM_I_WIDTH(7) < 100;
    }
};


#include "processor.h"
#include "mmu.h"
#include "simif.h"



class sim_wrap : public simif_t{
public:

    Memory memory;
    queue<IoAccess> mmioDut;

    // should return NULL for MMIO addresses
    virtual char* addr_to_mem(reg_t addr)  {
        if((addr & 0xE0000000) == 0x00000000) return NULL;
        return (char*)memory.get(addr);
    }
    // used for MMIO addresses
    virtual bool mmio_load(reg_t addr, size_t len, uint8_t* bytes)  {
//        printf("mmio_load %lx %ld\n", addr, len);
        if(addr < 0x10000000 || addr > 0x20000000) return false;
        assertTrue("missing mmio\n", !mmioDut.empty());
        auto dut = mmioDut.front();
        assertEq("mmio write\n", dut.write, false);
        assertEq("mmio address\n", dut.addr, addr);
        assertEq("mmio len\n", dut.len, len);
        memcpy(bytes, dut.data, len);
        mmioDut.pop();
        return !dut.error;
    }
    virtual bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes)  {
//        printf("mmio_store %lx %ld\n", addr, len);
        if(addr < 0x10000000 || addr > 0x20000000) return false;
        assertTrue("missing mmio\n", !mmioDut.empty());
        auto dut = mmioDut.front();
        assertEq("mmio write\n", dut.write, true);
        assertEq("mmio address\n", dut.addr, addr);
        assertEq("mmio len\n", dut.len, len);
        assertTrue("mmio data\n", !memcmp(dut.data, bytes, len));
        mmioDut.pop();
        return !dut.error;
    }
    // Callback for processors to let the simulation know they were reset.
    virtual void proc_reset(unsigned id)  {
//        printf("proc_reset %d\n", id);
    }

    virtual const char* get_symbol(uint64_t addr)  {
//        printf("get_symbol %lx\n", addr);
        return NULL;
    }
};

class RobCtx{
public:
    // RvData : u64
    RvData pc;
    bool integerWriteValid;
    RvData integerWriteData;
    bool floatWriteValid;
    RvFloat floatWriteData;
    int floatFlags;
    bool csrValid;
    bool csrWriteDone;
    bool csrReadDone;
    int  csrAddress;
    RvData csrWriteData;
    RvData csrReadData;

    IData branchHistory; //Todo what type?
    int opId;

    void clear(){
        integerWriteValid = false;
        floatWriteValid = false;
        csrValid = false;
        csrWriteDone = false;
        csrReadDone = false;
        floatFlags = 0;
    }
};


class FetchCtx{
public:
    u64 fetchAt;
    u64 decodeAt;
};


class OpCtx{
public:
    int fetchId;
    int robId;

    u64 pc;
    u64 instruction;
    u64 renameAt;
    u64 dispatchAt;
    u64 issueAt;
    u64 completeAt;
    u64 commitAt;
    u64 storeAt;
    u64 counter;
    bool sqAllocated;
    int sqId;

    void init(){
        renameAt = 0;
        dispatchAt = 0;
        issueAt = 0;
        completeAt = 0;
        commitAt = 0;
        storeAt = 0;
    }
};

class NaxStats{
public:
    u64 cycles = 0;
    u64 commits = 0;
    u64 reschedules = 0;
    u64 trap = 0;
    u64 branchMiss = 0;
    u64 jumpMiss = 0;
    u64 storeToLoadHazard = 0;
    u64 loadHitMissPredicted = 0;

    map<RvData, u64> branchMissHist;
    map<RvData, u64> jumpMissHist;
    map<RvData, u64> pcHist;

    string report(string tab, bool hist){
        stringstream ss;
        string ret = "";
        ss << tab << "IPC               " << (double)commits/cycles <<  endl;
        ss << tab << "cycles            " << cycles <<  endl;
        ss << tab << "commits           " << commits <<  endl;
        ss << tab << "reschedules       " << reschedules <<  endl;
        ss << tab << "trap              " << trap <<  endl;
        ss << tab << "branch miss       " << branchMiss <<  endl;
        ss << tab << "jump miss         " << jumpMiss <<  endl;
        ss << tab << "storeToLoadHazard " << storeToLoadHazard <<  endl;
        ss << tab << "loadHitMiss       " << loadHitMissPredicted << endl;
        if(hist){
            u64 branchCount = 0;
            ss << tab << "branch miss from :" << endl;
            for (auto const& x : branchMissHist){
                auto key = x.first;
                auto val = x.second;
                ss << tab << tab << hex << key <<" " << dec <<  setw(5) << val << " / " << pcHist[key] << endl;
                branchCount += pcHist[key];
            }
            ss << tab << "branch miss rate : " << (float)branchMiss/branchCount << endl;

            u64 jumpCount = 0;
            ss << tab << "jump miss from :" << endl;
            for (auto const& x : jumpMissHist){
                auto key = x.first;
                auto val = x.second;
                ss << tab << tab << hex << key <<" " << dec <<  std::setw(5) << val << " / " << pcHist[key] << endl;
                jumpCount += pcHist[key];
            }
            ss << tab << "jump miss rate : " << (float)jumpMiss/jumpCount << endl;

        }
        return ss.str();
    }
};

#define REASON_TRAP 0x01
#define LOAD_HIT_MISS_PREDICTED 0x03
#define REASON_BRANCH 0x10
#define REASON_JUMP 0x11
#define REASON_STORE_TO_LOAD_HAZARD 0x20


class NaxWhitebox : public SimElement{
public:

    VNaxRiscv_NaxRiscv* nax;

    //define ROB Table
    RobCtx robCtx[ROB_SIZE];
    //u64 fetchAt / u64 decodeAt
    FetchCtx fetchCtx[4096];
    //operation
    OpCtx opCtx[4096];
    queue <int> opIdInFlight;
    int sqToOp[256];
    RvData *robToPc[DISPATCH_COUNT];
    CData *integer_write_valid[INTEGER_WRITE_COUNT];
    CData *integer_write_robId[INTEGER_WRITE_COUNT];
#ifdef RVF
    CData *float_write_valid[FLOAT_WRITE_COUNT];
    CData *float_write_robId[FLOAT_WRITE_COUNT];
    RvFloat *float_write_data[FLOAT_WRITE_COUNT];
    CData *float_flags_robId[FPU_ROB_TO_FLAG_COUNT];
    CData *float_flags_mask[FPU_ROB_TO_FLAG_COUNT];
#endif
    CData *rob_completions_valid[ROB_COMPLETIONS_PORTS];
    CData *rob_completions_payload[ROB_COMPLETIONS_PORTS];
    CData *issue_valid[ISSUE_PORTS];
    CData *issue_robId[ISSUE_PORTS];
    CData *sq_alloc_valid[DISPATCH_COUNT];
    CData *sq_alloc_id[DISPATCH_COUNT];
    SData *decoded_fetch_id[DISPATCH_COUNT];
    CData *decoded_mask[DISPATCH_COUNT];
    SData *allocated_fetch_id[DISPATCH_COUNT];
    IData *decoded_instruction[DISPATCH_COUNT];
    RvData *decoded_pc[DISPATCH_COUNT];
    CData *dispatch_mask[DISPATCH_COUNT];
    RvData *integer_write_data[INTEGER_WRITE_COUNT];
    ofstream gem5;
    disassembler_t disasm;
    bool gem5Enable = false;

    u64 opCounter = 0;
    int periode = 2;

    bool statsCaptureEnable = true;
    NaxStats stats;

    NaxWhitebox(VNaxRiscv_NaxRiscv* nax): robToPc{MAP_INIT(&nax->robToPc_pc_,  DISPATCH_COUNT,)},
            integer_write_valid{MAP_INIT(&nax->integer_write_,  INTEGER_WRITE_COUNT, _valid)},
            integer_write_robId{MAP_INIT(&nax->integer_write_,  INTEGER_WRITE_COUNT, _robId)},
            integer_write_data{MAP_INIT(&nax->integer_write_,  INTEGER_WRITE_COUNT, _data)},
            #ifdef RVF
            float_write_valid{MAP_INIT(&nax->float_write_,  FLOAT_WRITE_COUNT, _valid)},
            float_write_robId{MAP_INIT(&nax->float_write_,  FLOAT_WRITE_COUNT, _robId)},
            float_write_data{MAP_INIT(&nax->float_write_,  FLOAT_WRITE_COUNT, _data)},
            float_flags_robId{MAP_INIT(&nax->fpuRobToFlags_,  FPU_ROB_TO_FLAG_COUNT, _robId)},
            float_flags_mask{MAP_INIT(&nax->fpuRobToFlags_,  FPU_ROB_TO_FLAG_COUNT, _mask)},
            #endif
            rob_completions_valid{MAP_INIT(&nax->RobPlugin_logic_whitebox_completionsPorts_,  ROB_COMPLETIONS_PORTS, _valid)},
            rob_completions_payload{MAP_INIT(&nax->RobPlugin_logic_whitebox_completionsPorts_,  ROB_COMPLETIONS_PORTS, _payload_id)},
            issue_valid{MAP_INIT(&nax->DispatchPlugin_logic_whitebox_issuePorts_,  ISSUE_PORTS, _valid)},
            issue_robId{MAP_INIT(&nax->DispatchPlugin_logic_whitebox_issuePorts_,  ISSUE_PORTS, _payload_robId)},
            sq_alloc_valid{MAP_INIT(&nax->sqAlloc_,  DISPATCH_COUNT, _valid)},
            sq_alloc_id{MAP_INIT(&nax->sqAlloc_,  DISPATCH_COUNT, _id)},
            decoded_fetch_id{MAP_INIT(&nax->FrontendPlugin_decoded_FETCH_ID_,  DISPATCH_COUNT,)},
            decoded_mask{MAP_INIT(&nax->FrontendPlugin_decoded_Frontend_DECODED_MASK_,  DISPATCH_COUNT,)},
            decoded_instruction{MAP_INIT(&nax->FrontendPlugin_decoded_Frontend_INSTRUCTION_DECOMPRESSED_,  DISPATCH_COUNT,)},
            decoded_pc{MAP_INIT(&nax->FrontendPlugin_decoded_PC_,  DISPATCH_COUNT,)},
            dispatch_mask{MAP_INIT(&nax->FrontendPlugin_dispatch_Frontend_DISPATCH_MASK_,  DISPATCH_COUNT,)},
            disasm(XLEN){
        this->nax = nax;
    }


    void traceGem5(bool enable){
        gem5Enable = enable;
    }

    string traceT2s(u64 time){
        return time ? std::to_string(time) : "";
    }
    void trace(int opId){
        auto &op = opCtx[opId];
        auto &fetch = fetchCtx[op.fetchId];
        string assembly = disasm.disassemble(op.instruction);
        gem5 << "O3PipeView:fetch:" << traceT2s(fetch.fetchAt) << ":0x" << hex <<  setw(8) << std::setfill('0') << op.pc << dec << ":0:" << op.counter << ":" << assembly << endl;
        gem5 << "O3PipeView:decode:"<< traceT2s(fetch.decodeAt) << endl;
        gem5 << "O3PipeView:rename:"<< traceT2s(op.renameAt) << endl;
        gem5 << "O3PipeView:dispatch:"<< traceT2s(op.dispatchAt) << endl;
        gem5 << "O3PipeView:issue:"<< traceT2s(op.issueAt) << endl;
        gem5 << "O3PipeView:complete:"<< traceT2s(op.completeAt) << endl;
        gem5 << "O3PipeView:retire:" << traceT2s(op.commitAt) << ":store:" << traceT2s(op.sqAllocated ? op.storeAt : 0) << endl;
//        assertTrue("a", fetch.fetchAt  <= fetch.decodeAt);
//        assertTrue("b", fetch.decodeAt <= op.renameAt);
//        assertTrue("c", op.renameAt    <= op.dispatchAt);
//        assertTrue("d", op.dispatchAt    <= op.issueAt);
//        assertTrue("e", op.issueAt    <= op.completeAt);
//        if(op.sqAllocated){
//            assertTrue("f", op.completeAt    <= op.commitAt);
//        }
    }

    virtual void onReset(){
        for(int i = 0;i < ROB_SIZE;i++){
            robCtx[i].clear();
        }
    }

    virtual void preCycle(){
        if(nax->robToPc_valid){
            for(int i = 0;i < DISPATCH_COUNT;i++){
                robCtx[nax->robToPc_robId + i].pc = *robToPc[i];
            }
        }

#ifdef RVF
        for(int i = 0;i < FPU_ROB_TO_FLAG_COUNT;i++){
            auto robId = *float_flags_robId[i];
            robCtx[robId].floatFlags |= *float_flags_mask[i];
        }
#endif

        for(int i = 0;i < INTEGER_WRITE_COUNT;i++){
            if(*integer_write_valid[i]){
                auto robId = *integer_write_robId[i];
//                printf("RF write rob=%d %d at %ld\n", robId, *integer_write_data[i], main_time);
                robCtx[robId].integerWriteValid = true;
                robCtx[robId].integerWriteData = *integer_write_data[i];
            }
        }

#ifdef RVF
        for(int i = 0;i < FLOAT_WRITE_COUNT;i++){
            if(*float_write_valid[i]){
                auto robId = *float_write_robId[i];
//                printf("RF write rob=%d %d at %ld\n", robId, *float_write_data[i], main_time);
                robCtx[robId].floatWriteValid = true;
                robCtx[robId].floatWriteData = *float_write_data[i];
            }
        }
#endif

        if(nax->FetchPlugin_stages_1_isFirstCycle){
            auto fetchId = nax->FetchPlugin_stages_1_FETCH_ID;
            fetchCtx[fetchId].fetchAt = main_time-periode*2;
        }


        if(nax->fetchLastFire){
            auto fetchId = nax->fetchLastId;
            fetchCtx[fetchId].decodeAt = main_time;
        }

        for(int i = 0;i < DISPATCH_COUNT;i++){
            if(nax->FrontendPlugin_decoded_isFireing){
                auto fetchId = *decoded_fetch_id[i];
                auto opId = nax->FrontendPlugin_decoded_OP_ID + i;
                if(*decoded_mask[i]) opIdInFlight.push(opId);
                opCtx[opId].init();
                opCtx[opId].fetchId = fetchId;
                opCtx[opId].renameAt = main_time;
                opCtx[opId].instruction = *decoded_instruction[i];
                opCtx[opId].pc = *decoded_pc[i];
            }
            if(nax->FrontendPlugin_allocated_isFireing){
                auto robId = nax->FrontendPlugin_allocated_ROB_ID + i;
                auto opId = nax->FrontendPlugin_allocated_OP_ID + i;
                robCtx[robId].opId = opId;
                opCtx[opId].robId = robId;
            }
        }

        if (nax->FrontendPlugin_dispatch_isFireing) {
            for (int i = 0; i < DISPATCH_COUNT; i++) {
                if (*dispatch_mask[i]) {
                    auto robId = nax->FrontendPlugin_dispatch_ROB_ID + i;
                    auto opId = robCtx[robId].opId;
                    auto sqId = *sq_alloc_id[i];
                    opCtx[opId].dispatchAt = main_time;
                    opCtx[opId].sqAllocated = *sq_alloc_valid[i];
                    opCtx[opId].sqId = sqId;
                    if (*sq_alloc_valid[i]) {
                        sqToOp[sqId] = opId;
                    }
                }
            }
        }


        for(int i = 0;i < ISSUE_PORTS;i++){
            if(*issue_valid[i]){
                opCtx[robCtx[*issue_robId[i]].opId].issueAt = main_time;
            }
        }

        for(int i = 0;i < ROB_COMPLETIONS_PORTS;i++){
            if(*rob_completions_valid[i]){
                opCtx[robCtx[*rob_completions_payload[i]].opId].completeAt = main_time;
            }
        }

        for(int i = 0;i < COMMIT_COUNT;i++){
            if((nax->commit_mask >> i) & 1){
                auto robId = nax->commit_robId + i;
                auto opId = robCtx[robId].opId;
                opCtx[opId].commitAt = main_time;
                while(true){
                    auto front = opIdInFlight.front();
                    opIdInFlight.pop();
                    opCtx[front].counter = opCounter++;
                    if(gem5Enable/* && !opCtx[opId].sqAllocated*/) trace(front);
                    if(front == opId) break;
                }
            }
        }
//        if(nax->sqFree_valid){
//            auto opId = sqToOp[nax->sqFree_payload];
//            assertTrue("??? at sqFree", opCtx[opId].sqAllocated);
//            opCtx[opId].storeAt = main_time;
//            if(gem5Enable) trace(opId);
//        }
//        if(nax->reschedule_valid){
//            for(int robId = nax->reschedule_payload_robId + !nax->reschedule_payload_skipCommit; robId < nax->reschedule_payload_robIdNext;robId = (robId + 1) % ROB_SIZE){
//
//                auto opId = robCtx[robId].opId;
//                trace(opId)
//            }
//        }
        if(nax->csrAccess_valid){
            auto robId = nax->csrAccess_payload_robId;
//                printf("RF write rob=%d %d at %ld\n", robId, *integer_write_data[i], main_time);
            robCtx[robId].csrValid = true;
            robCtx[robId].csrAddress = nax->csrAccess_payload_address;
            robCtx[robId].csrWriteDone = nax->csrAccess_payload_writeDone;
            robCtx[robId].csrReadDone = nax->csrAccess_payload_readDone;
            robCtx[robId].csrWriteData = nax->csrAccess_payload_write;
            robCtx[robId].csrReadData = nax->csrAccess_payload_read;
        }

//        if(nax->FrontendPlugin_allocated_isFireing){
//            auto robId = nax->FrontendPlugin_allocated_ROB_ID;
//            robCtx[robId].branchHistory = nax->FrontendPlugin_allocated_BRANCH_HISTORY_0;
//        }
//        for(int i = 0;i < COMMIT_COUNT;i++){
//            if((nax->commit_mask >> i) & 1){
//                auto robId = nax->commit_robId + i;
//                if(nax->HistoryPlugin_logic_onCommit_value != robCtx[robId].branchHistory) {
//                    printf("!! %ld %x %x\n", main_time, nax->HistoryPlugin_logic_onCommit_value, robCtx[robId].branchHistory);
//                    failure();
//                }
//            }
//        }
        if(statsCaptureEnable){
            stats.cycles += 1;
            for(int i = 0;i < COMMIT_COUNT;i++){
                if((nax->commit_mask >> i) & 1){
                    auto robId = nax->commit_robId + i;
                    RvData pc = robCtx[robId].pc;
                    stats.commits += 1;
                    stats.pcHist[pc] += 1;
//                    if(pc == 0x80001ed0) printf("PC commit at %ld\n", main_time);
                }
            }
            if(nax->reschedule_valid){
                RvData pc = robCtx[nax->reschedule_payload_robId].pc;
                stats.reschedules += 1;
                switch(nax->rescheduleReason){
                case REASON_TRAP: stats.trap += 1; break;
                case REASON_BRANCH: {
                    stats.branchMiss += 1;
                    stats.branchMissHist[pc] += 1;
                } break;
                case REASON_JUMP: {
                    stats.jumpMiss += 1;
                    stats.jumpMissHist[pc] += 1;
                } break;
                case REASON_STORE_TO_LOAD_HAZARD: stats.storeToLoadHazard += 1; break;
                case LOAD_HIT_MISS_PREDICTED: stats.loadHitMissPredicted += 1; break;
                }
            }
        }

    }

    virtual void postCycle(){

    }
};

#ifdef ALLOCATOR_CHECKS
#include "VNaxRiscv_AllocatorMultiPortMem.h"

class NaxAllocatorChecker : public SimElement{
public:
    VNaxRiscv_AllocatorMultiPortMem* dut;
    bool busy[INTEGER_PHYSICAL_DEPTH];
    CData *push_valid[COMMIT_COUNT];
    CData *push_payload[COMMIT_COUNT];
    CData *pop_values[DISPATCH_COUNT];

    NaxAllocatorChecker(VNaxRiscv_NaxRiscv* nax): dut(nax->integer_RfAllocationPlugin_logic_allocator),
        push_valid{MAP_INIT(&dut->io_push_,  COMMIT_COUNT, _valid)},
        push_payload{MAP_INIT(&dut->io_push_,  COMMIT_COUNT, _payload)},
        pop_values{MAP_INIT(&dut->io_pop_values_,  DISPATCH_COUNT, )} {
    }


    virtual void onReset(){
        for(int i = 0;i < INTEGER_PHYSICAL_DEPTH;i++){
            busy[i] = true;
        }
    }

    virtual void preCycle(){
        for(int i = 0;i < COMMIT_COUNT;i+=1){
            if(*push_valid[i]){
                int phys = *push_payload[i];
                assertTrue("Double free", busy[phys]);
                busy[phys] = false;
            }
        }
        if(dut->io_pop_fire){
            for(int i = 0;i < DISPATCH_COUNT;i+=1){
                if(CHECK_BIT(dut->io_pop_mask,i)){
                    int phys = *pop_values[i];
                    assertTrue("Double alloc", !busy[phys]);
                    busy[phys] = true;
                }
            }
        }
    }

    virtual void postCycle(){

    }
};

#endif



int syncSockfd, syncConnfd;


//http://www.mario-konrad.ch/blog/programming/getopt.html
enum ARG
{
    ARG_LOAD_HEX = 1,
    ARG_LOAD_ELF,
    ARG_LOAD_BIN,
    ARG_LOAD_U32,
    ARG_START_SYMBOL,
    ARG_START_ADD,
    ARG_PASS_SYMBOL,
    ARG_FAIL_SYMBOL,
    ARG_OUTPUT_DIR,
    ARG_NAME,
    ARG_TIMEOUT,
    ARG_PROGRESS,
    ARG_SEED,
    ARG_TRACE,
    ARG_TRACE_START_TIME,
    ARG_TRACE_STOP_TIME,
    ARG_TRACE_START_PC,
    ARG_TRACE_STOP_PC,
    ARG_TRACE_SPORADIC,
    ARG_TRACE_REF,
    ARG_STATS_PRINT,
    ARG_STATS_PRINT_ALL,
    ARG_STATS_START_SYMBOL,
    ARG_STATS_STOP_SYMBOL,
    ARG_STATS_TOGGLE_SYMBOL,
    ARG_TRACE_GEM5,
    ARG_SPIKE_DEBUG,
    ARG_SPIKE_DISABLED,
    ARG_TIMEOUT_DISABLED,
    ARG_MEMORY_LATENCY,
    ARG_BLOCK_DEVICE,
    ARG_FRAMEBUFFER,
    ARG_SIM_MASTER,
    ARG_SIM_SLAVE,
    ARG_SIM_SLAVE_DELAY,
    ARG_NO_STDIN,
    ARG_NO_PUTC_FLUSH,
    ARG_PUTC_TIMESTAMP,
    ARG_PUTC,
    ARG_GETC,
    ARG_SUCCESS,
    ARG_HELP,
};


static const struct option long_options[] =
{
    { "help", no_argument, 0, ARG_HELP },
    { "load-hex", required_argument, 0, ARG_LOAD_HEX },
    { "load-elf", required_argument, 0, ARG_LOAD_ELF },
    { "load-bin", required_argument, 0, ARG_LOAD_BIN },
    { "load-u32", required_argument, 0, ARG_LOAD_U32 },
    { "start-symbol", required_argument, 0, ARG_START_SYMBOL },
    { "start-add", required_argument, 0, ARG_START_ADD },
    { "pass-symbol", required_argument, 0, ARG_PASS_SYMBOL },
    { "fail-symbol", required_argument, 0, ARG_FAIL_SYMBOL },
    { "output-dir", required_argument, 0, ARG_OUTPUT_DIR },
    { "name", required_argument, 0, ARG_NAME },
    { "timeout", required_argument, 0, ARG_TIMEOUT },
    { "progress", required_argument, 0, ARG_PROGRESS },
    { "seed", required_argument, 0, ARG_SEED },
    { "trace", no_argument, 0, ARG_TRACE },
    { "trace-start-time", required_argument, 0, ARG_TRACE_START_TIME },
    { "trace-stop-time", required_argument, 0, ARG_TRACE_STOP_TIME },
    { "trace-start-pc", required_argument, 0, ARG_TRACE_START_PC },
    { "trace-stop-pc", required_argument, 0, ARG_TRACE_STOP_PC },
    { "trace-sporadic", required_argument, 0, ARG_TRACE_SPORADIC },
    { "trace-ref", no_argument, 0, ARG_TRACE_REF },
    { "stats-print", no_argument, 0, ARG_STATS_PRINT },
    { "stats-print-all", no_argument, 0, ARG_STATS_PRINT_ALL },
    { "stats-start-symbol", required_argument, 0, ARG_STATS_START_SYMBOL },
    { "stats-stop-symbol", required_argument, 0, ARG_STATS_STOP_SYMBOL },
    { "stats-toggle-symbol", required_argument, 0, ARG_STATS_TOGGLE_SYMBOL },
    { "trace-gem5", no_argument, 0, ARG_TRACE_GEM5 },
    { "spike-debug", no_argument, 0, ARG_SPIKE_DEBUG },
    { "spike-disabled", no_argument, 0, ARG_SPIKE_DISABLED},
    { "timeout-disabled", no_argument, 0, ARG_TIMEOUT_DISABLED},
    { "memory-latency", required_argument, 0, ARG_MEMORY_LATENCY },
    { "block-device", required_argument, 0, ARG_BLOCK_DEVICE },
    { "framebuffer", required_argument, 0, ARG_FRAMEBUFFER },
    { "sim-master", no_argument, 0, ARG_SIM_MASTER },
    { "sim-slave", no_argument, 0, ARG_SIM_SLAVE },
    { "sim-slave-delay", required_argument, 0, ARG_SIM_SLAVE_DELAY },
    { "no-stdin", no_argument, 0, ARG_NO_STDIN },
    { "no-putc-flush", no_argument, 0, ARG_NO_PUTC_FLUSH },
    { "putc-timestamp", no_argument, 0, ARG_PUTC_TIMESTAMP },
    { "putc", required_argument, 0, ARG_PUTC },
    { "getc", required_argument, 0, ARG_GETC },
    { "success", no_argument, 0, ARG_SUCCESS },
    0
};


string helpString = R"(
--help                  : Print this

Simulation setup
--load-bin=FILE,ADDRESS : Load a binary file in the simulation memory at the given hexadecimal address. ex file,80000000
--load-hex=FILE         : Load a hex file in the simulation memory
--load-elf=FILE         : Load a elf file in the simulation memory
--start-symbol=SYMBOL   : Force the CPU to boot at the given elf symbol
--pass-symbol=SYMBOL    : The simulation will pass when the given elf symbol execute
--fail-symbol=SYMBOL    : The simulation will fail when the given elf symbol execute
--timeout=INT           : Simulation time before failure (~number of cycles x 2)
--seed=INT              : Seed used to initialize randomizers
--memory-latency=CYCLES : Specify the minimal memory latency from cmd to the first rsp beat
--no-stdin              : Do not redirect the terminal stdin to the simulated getc
--no-putc-flush         : The sim will not flush the terminal stdout after each sim putc
--name=STRING           : Test name reported when on exit (not very useful XD)

Simulation tracing / probing
--output-dir=DIR        : Path to where every traces will be written
--progress=PERIOD       : Will print the simulation speed each period seconds
--trace                 : Enable FST wave capture
--trace-start-time=INT  : Add a time to which the FST should start capturing
--trace-stop-time=INT   : Add a time to which the FST should stop capturng
--trace-sporadic=RATIO  : Specify that periodically the FST capture a bit of the wave
--trace-ref             : Store the spike execution traces in a file
--stats-print           : Print some stats about the CPU execution at the end of the sim
--stats-print-all       : Print all the stats possible (including which branches had miss)
--stats-start-symbol=SY : Specify at which elf symbol the stats should start capturing
--stats-stop-symbol=SYM : Specify at which elf symbol the stats should stop capturing
--stats-toggle-symbol=S : Specify at which elf symbol the stats should change its capture state
--trace-gem5            : Enable capture of the pipeline timings as a gem5 trace, readable with github konata
--spike-debug           : Enable spike debug mode (more verbose traces)
--sim-master            : The simulation will wait a sim-slave to connect and then run until pass/fail
--sim-slave             : The simulation will connect to a sim-master and then run behind it
                          When the sim-master fail, then the sim-slave will run to that point with trace enabled
--sim-slave-delay=TIME  : For the sim-slave, specify how much behind the sim-master it has to be.

Directed test argument : Used, for instance, to automate the shell interactions in the linux regression
--putc=STRING          : Send the given string to the sim getc
--getc=STRING          : Wait the sim to putc the given string
--success              : Quit the simulation successfully
)";

u64 startPc = 0x80000000l;
map<RvData, vector<function<void(RvData)>>> pcToEvent;
map<u64, vector<function<void()>>> timeToEvent;
void addPcEvent(RvData pc, function<void(RvData)> func){
    if(pcToEvent.count(pc) == 0) pcToEvent[pc] = vector<function<void(RvData)>>();
    pcToEvent[pc].push_back(func);
}

void addTimeEvent(u64 time, function<void()> func){
    if(timeToEvent.count(time) == 0) timeToEvent[time] = vector<function<void()>>();
    timeToEvent[time].push_back(func);
}


bool trace_enable = true;
float trace_sporadic_factor = 0.0f;
u64 trace_sporadic_period = 200000;
u64 trace_sporadic_trigger;
processor_t *proc;
sim_wrap *wrap;
state_t *state;
FILE *fptr;
VNaxRiscv *top;
NaxWhitebox *whitebox;
vector<SimElement*> simElements;
//map<RvData, u64> pageFaultSinceFenceVma;

u64 statsStartAt = -1;
u64 statsStopAt = -1;
u64 statsToggleAt = -1;

#ifdef TRACE
VerilatedFstC* tfp;
#endif

VNaxRiscv_NaxRiscv *topInternal;
u64 traps_since_commit = 0;
u64 commits = 0;
u64 last_commit_pc;
int robIdChecked = 0;
int cycleSinceLastCommit = 0;
std::chrono::high_resolution_clock::time_point progressLast;
vluint64_t progressMainTimeLast = 0;

u64 simSlaveTraceDuration = 100000;
u64 simMasterTime = 0;
bool simMasterFailed = false;



void parseArgFirst(int argc, char** argv){
    while (1) {
        int index = -1;
        struct option * opt = 0;
        int result = getopt_long(argc, argv,"abc:d", long_options, &index);
        if (result == -1) break;
        switch (result) {
            case ARG_SEED: {
                //set random seed
                Verilated::randSeed(stoi(optarg));
                srand48(stoi(optarg));
            } break;

            case ARG_TRACE: {
                //set Trace
                traceWave = true;
#ifndef TRACE
                printf("You need to recompile with TRACE=yes to enable tracing"); failure();
#endif
            } break;
            case ARG_TRACE_START_TIME: trace_enable = false; addTimeEvent(stol(optarg), [&](){ trace_enable = true;}); break;
            case ARG_TRACE_STOP_TIME: trace_enable = false; addTimeEvent(stol(optarg), [&](){ trace_enable = false;}); break;
            case ARG_TRACE_START_PC: {
                u64 value;
                if(sscanf(optarg, "%lx", &value) == EOF) {
                    cout << "Bad ARG_TRACE_START_PC formating" << endl;
                    failure()
                }

                trace_enable = false;
                addPcEvent(value, [&](RvData pc){  trace_enable = true; });
            }break;
            case ARG_TRACE_STOP_PC: {
                u64 value;
                if(sscanf(optarg, "%lx", &value) == EOF) {
                    cout << "Bad ARG_TRACE_STOP_PC formating" << endl;
                    failure()
                }
                trace_enable = false;
                addPcEvent(value, [&](RvData pc){  trace_enable = false; });
            }break;
            /*
             * ARG_TRACE_SPORADIC:设置采样率
             * ARG_PROGRESS:隔多少周期输出一次仿真报告
             */
            case ARG_TRACE_SPORADIC: trace_enable = false; trace_sporadic_factor = stof(optarg); break;
            case ARG_TRACE_REF: trace_ref = true; break;
            case ARG_NAME: simName = optarg; break;
            case ARG_OUTPUT_DIR: outputDir = optarg; break;
            case ARG_TIMEOUT: timeout = stoi(optarg); break;
            case ARG_PROGRESS: progressPeriod = stod(optarg); break;
            case ARG_STATS_PRINT: statsPrint = true; break;
            case ARG_STATS_PRINT_ALL: statsPrint = true; statsPrintHist = true; break;
            case ARG_TRACE_GEM5: traceGem5 = true; break;
            case ARG_HELP: cout << helpString; exit(0); break;
            case ARG_SPIKE_DEBUG: spike_debug = true; break;
            case ARG_SPIKE_DISABLED : spike_enabled = false; break;
            case ARG_TIMEOUT_DISABLED : timeout_enabled = false; break;
            case ARG_SIM_MASTER: simMaster = true; break;
            case ARG_SIM_SLAVE: simSlave = true; trace_enable = false; break;
            case ARG_SIM_SLAVE_DELAY: simSlaveTraceDuration = stol(optarg); break;
            case ARG_NO_PUTC_FLUSH: putcFlush = false;  break;
            case ARG_PUTC_TIMESTAMP: putcTimestamp = true;  break;
            case ARG_GETC: testScheduleQueue.push(new WaitPutc(string(optarg))); break;
            case ARG_PUTC: testScheduleQueue.push(new DoGetc(string(optarg))); break;
            case ARG_SUCCESS: testScheduleQueue.push(new DoSuccess()); break;
            case ARG_NO_STDIN: noStdIn = true; break;
            case ARG_BLOCK_DEVICE:
            case ARG_FRAMEBUFFER:
            case ARG_MEMORY_LATENCY:
            case ARG_LOAD_HEX:
            case ARG_LOAD_ELF:
            case ARG_LOAD_BIN:
            case ARG_LOAD_U32:
            case ARG_START_SYMBOL:
            case ARG_START_ADD:
            case ARG_PASS_SYMBOL:
            case ARG_FAIL_SYMBOL:
            case ARG_STATS_START_SYMBOL:
            case ARG_STATS_STOP_SYMBOL:
            case ARG_STATS_TOGGLE_SYMBOL: break;
            default: {
                printf("Unknown argument\n");
                failure();
                break;
            }
        }
    }

    trace_sporadic_trigger = trace_sporadic_period * trace_sporadic_factor;
    mkpath(outputDir, 0777);
}

void parseArgsSecond(int argc, char** argv){
    u32 nop32 = 0x13;
    u8 *nop = (u8 *)&nop32;
    Elf *elf = NULL;
    optind = 1;
    while (1) {
        int index = -1;
        struct option * opt = 0;
        int result = getopt_long(argc, argv,"abc:d", long_options, &index);
        if (result == -1) break;
        switch (result) {
            case ARG_LOAD_HEX: wrap->memory.loadHex(string(optarg)); soc->memory.loadHex(string(optarg)); break;
            case ARG_LOAD_ELF: {
                elf = new Elf(optarg);
                elf->visitBytes([&](u8 data, u64 address) {
                    wrap->memory.write(address, 1, &data);
                    soc->memory.write(address, 1, &data);
                });
            }break;
            case ARG_LOAD_BIN: {
                u64 address;
                char path[201];
                if(sscanf(optarg, "%[^','],%lx", path, &address) == EOF) {
                    cout << "Bad load bin formating" << endl;
                    failure()
                }

                wrap->memory.loadBin(string(path), address);
                soc->memory.loadBin(string(path), address);
            }break;
            case ARG_LOAD_U32: {
                u64 address;
                u32 data;
                if(sscanf(optarg, "%x,%lx", &data, &address) == EOF) {
                    cout << "Bad load bin formating" << endl;
                    failure()
                }

                wrap->memory.write(address,4, (uint8_t*)&data);
                soc->memory.write(address,4, (uint8_t*)&data);
            }break;
            case ARG_START_SYMBOL: startPc = elf->getSymbolAddress(optarg); break;
            case ARG_START_ADD: startPc += stol(optarg); break;
            case ARG_PASS_SYMBOL: {
                u64 addr = elf->getSymbolAddress(optarg);
                addPcEvent(addr, [&](RvData pc){ success();});
                wrap->memory.write(addr, 4, nop);
                soc->memory.write(addr, 4, nop);
            }break;
            case ARG_FAIL_SYMBOL:  {
                u64 addr = elf->getSymbolAddress(optarg);
                addPcEvent(addr, [&](RvData pc){
                  printf("Failure due to fail symbol encounter\n");
                  failure();
                });
                wrap->memory.write(addr, 4, nop);
                soc->memory.write(addr, 4, nop);
            }break;
            case ARG_STATS_TOGGLE_SYMBOL: statsToggleAt = elf->getSymbolAddress(optarg); whitebox->statsCaptureEnable = false; break;
            case ARG_STATS_START_SYMBOL: statsStartAt = elf->getSymbolAddress(optarg); whitebox->statsCaptureEnable = false; break;
            case ARG_STATS_STOP_SYMBOL: statsStopAt = elf->getSymbolAddress(optarg); break;
            case ARG_MEMORY_LATENCY:{
                for(auto e : simElements){
                    if(auto v = dynamic_cast<WithMemoryLatency*>(e)) {
                       v->setLatency(stoi(optarg));
                    }
                }
            }break;
            case ARG_BLOCK_DEVICE:{
                char path[201];
                char wr[201];
                u64 capacity, mappingStart;
                if(sscanf(optarg, "%[^','],%[^','],%lx,%lx", path, wr, &capacity, &mappingStart) == EOF) {
                    cout << "Bad block device arg formating" << endl;
                    failure()
                }
                auto e = new SimpleBlockDevice(path, strcmp("wr", wr) == 0, capacity, mappingStart);
                soc->socElements.push_back(e);
            } break;
            case ARG_FRAMEBUFFER:{
                u32 width, height, startAt;
                if(sscanf(optarg, "%x,%d,%d", &startAt, &width, &height) == EOF) {
                    cout << "Bad framebuffer arg formating" << endl;
                    failure()
                }
                u32 endAt = startAt + width*height*4;
                auto fb = new Framebuffer(width, height);
                soc->snoopWrites.push_back([startAt, endAt, fb](u32 address,u32 length, u8 *data){
                    if(address >= startAt && address+length <= endAt){
                        memcpy((u8*)fb->pixels + (address-startAt), data, length);
                    }
                });
                simElements.push_back(fb);
            } break;
            default:  break;
        }
    }
    /* print all other parameters */
    while (optind < argc)
    {
        printf("other parameter: <%s>\n", argv[optind++]);
    }


    #ifdef TRACE
    if(traceWave){
        tfp = new VerilatedFstC;
        top->trace(tfp, 99);
        tfp->open((outputDir + "/wave.fst").c_str());
    }
    #endif


    state->pc = startPc;
    progressLast = std::chrono::high_resolution_clock::now();
}

void verilatorInit(int argc, char** argv){
    /*
     * Verilated::debug:set debug level（调试级别）
     * Verilated::randReset : 随机数种子
     * Verilated::traceEverOn: 信号变化日志
     * Verilated::commandArgs ：传递命令行
     * Verilated::mkdir : mkdir
     */
    Verilated::debug(0);
    Verilated::randReset(2);
    Verilated::traceEverOn(true);
    Verilated::commandArgs(argc, argv);
    Verilated::mkdir("logs");
}

void spikeInit(){
    /*
     * Spike创建处理器实例的代码
     */
    fptr = trace_ref ? fopen((outputDir + "/spike.log").c_str(),"w") : NULL;
    std::ofstream outfile ("/dev/null",std::ofstream::binary);
    wrap = new sim_wrap();
    string isa;
    #if XLEN==32
    isa += "RV32I";
    #else
    isa += "RV64I";
    #endif
    isa += "MA";
    #ifdef RVF
    isa += "F";
    #endif
    #ifdef RVD
    isa += "D";
    #endif
    if(RVC) isa += "C";
    /*
     * processor_t（指令集架构 / 处理器名字 / 。/。/处理器启动时间/是否立即启动/spike的配置选项/输出文件）
     */
    proc = new processor_t(isa.c_str(), "MSU", "", wrap, 0, false, fptr, outfile);
    proc->set_impl(IMPL_MMU_SV32, XLEN == 32);
    proc->set_impl(IMPL_MMU_SV39, XLEN == 64);
    proc->set_impl(IMPL_MMU_SV48, false);
    proc->set_impl(IMPL_MMU, true);
    if(spike_debug) proc->debug = true;
    proc->set_pmp_num(0);
    state = proc->get_state();
    for(int i = 0;i < 32;i++){
        float128_t tmp;
        tmp.v[0] = -1;
        tmp.v[1] = -1;
        state->FPR.write(i, tmp);
    }
}

void rtlInit(){
    //Todo about the NaxWhitebox
    top = new VNaxRiscv;  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
    topInternal = top->NaxRiscv;
    whitebox = new NaxWhitebox(top->NaxRiscv);
    whitebox->traceGem5(traceGem5);
    if(traceGem5) whitebox->gem5 = ofstream(outputDir + "/trace.gem5o3",std::ofstream::binary);

    soc = new Soc(top);
    /*simElemnets : 模拟的几个部件，vector<SimElement*> simElements;
     * onReset() postReset() preCycle() postCycle()
     *
     * soc类 : 初始化内存,可对内存数据进行读写，支持对外设的读写
     * 对外设进行写的时候，直接根据address，如果选中PUTC，直接printf
     * 读的时候，在soc对象内部存在一个，queue <char> customCin;出队即可读出
     * 顶层的soc对象 +
     * 模拟读写 Icahce / Dcache / 外部设备
     * FetchCached : 模拟从ICache中取指令的过程,取指令的延时 +2，并且没有考虑分支预测失败的的情况
     * DataCached: 分读写通道的时延计算 +2，通道填满后请求就可发出
     * LsuPeripheral: 模拟对外设数据的读写
     * whitebox:
     */
    simElements.push_back(soc);
    simElements.push_back(new FetchCached(top, soc, true));
    simElements.push_back(new DataCached(top, soc, true));

    //模拟内存的读写
    simElements.push_back(new LsuPeripheral(top, soc, &wrap->mmioDut, true));
    simElements.push_back(whitebox);

#ifdef EMBEDDED_JTAG
    simElements.push_back(new Jtag(
        &top->EmbeddedJtagPlugin_logic_jtag_tms,
        &top->EmbeddedJtagPlugin_logic_jtag_tdi,
        &top->EmbeddedJtagPlugin_logic_jtag_tdo,
        &top->EmbeddedJtagPlugin_logic_jtag_tck,
        2
    ));
#endif
#ifdef ALLOCATOR_CHECKS
    simElements.push_back(new NaxAllocatorChecker(top->NaxRiscv));
#endif
}

void simMasterSlaveInit(){
    //https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/   <3

    if(simMaster){
        struct sockaddr_in servaddr, cli;
        socklen_t len;

        // socket create and verification
        syncSockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (syncSockfd == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(SIM_MASTER_PORT);

        int one = 1;
        setsockopt(syncSockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

        // Binding newly created socket to given IP and verification
        if ((bind(syncSockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
            printf("socket bind failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully binded..\n");

        // Now server is ready to listen and verification
        if ((listen(syncSockfd, 5)) != 0) {
            printf("Listen failed...\n");
            exit(0);
        }
        else
            printf("Server listening..\n");
        len = sizeof(cli);

        // Accept the data packet from client and verification
        syncConnfd = accept(syncSockfd, (SA*)&cli, &len);
        if (syncConnfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");
    }

    if(simSlave){
        struct sockaddr_in servaddr, cli;
        socklen_t len;

        // socket create and varification
        syncSockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (syncSockfd == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        servaddr.sin_port = htons(SIM_MASTER_PORT);

        // connect the client socket to server socket
        if (connect(syncSockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            exit(0);
        }
        else
            printf("connected to the server..\n");
    }

}

void spikeStep(RobCtx & robCtx){
    //Sync some CSR
    state->mip->unlogged_write_with_mask(-1, 0);
    u64 backup;
    if(robCtx.csrReadDone){
        switch(robCtx.csrAddress){
        case MIP:
        case SIP:
        case UIP:
            backup = state->mie->read();
            state->mip->unlogged_write_with_mask(-1, robCtx.csrReadData);
            state->mie->unlogged_write_with_mask(MIE_MTIE | MIE_MEIE |  MIE_MSIE | MIE_SEIE, 0);
//                                cout << main_time << " " << hex << robCtx.csrReadData << " " << state->mip->read()  << " " << state->csrmap[robCtx.csrAddress]->read() << dec << endl;
            break;
        case CSR_MCYCLE:
        case CSR_UCYCLE:
            backup = state->minstret->read();
            state->minstret->unlogged_write(robCtx.csrReadData+1); //+1 patch a spike internal workaround XD
            break;
        case CSR_MCYCLEH:
        case CSR_UCYCLEH:
            backup = state->minstret->read();
            state->minstret->unlogged_write((((u64)robCtx.csrReadData) << 32)+1);
            break;
        default:
            if(robCtx.csrAddress >= CSR_MHPMCOUNTER3 && robCtx.csrAddress <= CSR_MHPMCOUNTER31){
                state->csrmap[robCtx.csrAddress]->unlogged_write(robCtx.csrReadData);
            }
            break;
        }
    }

    //Run spike for one commit or trap
    proc->step(1);
    state->mip->unlogged_write_with_mask(-1, 0);

    //Sync back some CSR
    if(robCtx.csrReadDone){
        switch(robCtx.csrAddress){
        case MIP:
        case SIP:
        case UIP:
            state->mie->unlogged_write_with_mask(MIE_MTIE | MIE_MEIE |  MIE_MSIE | MIE_SEIE, backup);
            break;
        case CSR_MCYCLE:
        case CSR_MCYCLEH:
            state->minstret->unlogged_write(backup+2);
            break;
            break;
        }
    }
}

void spikeSyncTrap(){
    if(top->NaxRiscv->trap_fire){
        bool interrupt = top->NaxRiscv->trap_interrupt;
        int code = top->NaxRiscv->trap_code;
        bool pageFault = !interrupt && (code == 12 || code == 13 || code == 15);
        int mask = 1 << code;

        if(pageFault){
            auto mmu = proc->get_mmu();
            mmu->flush_tlb();
            mmu->fault_fetch = code == 12;
            mmu->fault_load  = code == 13;
            mmu->fault_store = code == 15;
            mmu->fault_address = ((((s64)top->NaxRiscv->trap_tval) << (64-TVAL_WIDTH)) >> (64-TVAL_WIDTH));
        }
        if(interrupt) state->mip->write_with_mask(mask, mask);
        proc->step(1);
        if(interrupt) state->mip->write_with_mask(mask, 0);
        if(pageFault){
            auto mmu = proc->get_mmu();
            mmu->fault_fetch = false;
            mmu->fault_load  = false;
            mmu->fault_store = false;
        }

        traps_since_commit += 1;
        if(traps_since_commit > 10){
            cout << "DUT is blocked in a endless trap cycle of death" << endl;
            failure();
        }
    }
}

#define simMasterWrite(buf) assert(write(syncConnfd, &buf, sizeof(buf)) == sizeof(buf));
#define simMasterRead(buf) assert(read(syncSockfd, &buf, sizeof(buf)) == sizeof(buf));

enum SIM_MS_ENUM
{
    SIM_MS_TIME = 1,
    SIM_MS_FAIL,
    SIM_MS_PASS,
    SIM_MS_GETC,
};

void simMasterWriteHeader(SIM_MS_ENUM e) {
    simMasterWrite(e);
}
/*
 * 将当前仿真时间发给主进程进行同步
 */
void simMasterMainTime(){
    char buf[1+sizeof(main_time)];
    buf[0] = SIM_MS_TIME;
    simMasterWriteHeader(SIM_MS_TIME);
    simMasterWrite(main_time);
}

void simMasterGetC(char c){
    simMasterMainTime();
    simMasterWriteHeader(SIM_MS_GETC);
    simMasterWrite(c);
}

void simSlaveTick(){
    if(simMasterFailed) {
        if(main_time > simMasterTime){
            cout << "ERROR Slave sim is going futher than the master one ?????" << endl;
            failure();
        }
        return;
    }
    while(main_time + simSlaveTraceDuration >= simMasterTime){
        SIM_MS_ENUM header;
        simMasterRead(header);
        switch(header){
            case SIM_MS_TIME:{
                simMasterRead(simMasterTime);
            }break;
            case SIM_MS_FAIL:{
                simMasterFailed = true;
                auto time = simMasterTime - simSlaveTraceDuration;
                if(time > simMasterTime) time = main_time + 1;
                addTimeEvent(time, [&](){ trace_enable = true;});
                return;
            }break;
            case SIM_MS_PASS:{
                success();
            }break;
            case SIM_MS_GETC:{
                char c;
                simMasterRead(c);
                addTimeEvent(simMasterTime-1, [c](){ soc->customCin.push(c);});
            }break;
            default: {
                cout << "Unknown sim master header" << endl;
                failure();
            }break;
        }
    }
}


void simLoop(){
    try {
        top->clk = 0;

        //将仿真的模块先进行复位
        for(SimElement* simElement : simElements) simElement->onReset();

        //请求队列，模拟getc、putc、and quit the simulation successfully
        testScheduleQueueNext();


        while (!Verilated::gotFinish()) {
            if(simMaster && main_time % 50000 == 0){
                //仿真时间同步
                simMasterMainTime();
            }

            ++main_time;

            //控制从属进程的仿真速度 Todo
            if(simSlave) simSlaveTick();

            if(main_time == timeout){
                printf("simulation timeout\n");
                failure();
            }

            //触发对应的事件函数
            if(timeToEvent.count(main_time) != 0){
                for(auto event : timeToEvent[main_time]){
                    event();
                }
            }

            //spike启用日志提交功能
            if(trace_ref && proc->get_log_commits_enabled() != trace_enable){
                if(trace_enable){
                    proc->enable_log_commits();
                } else {
                    proc->disable_log_commits();
                }
            }

            #ifdef TRACE
            if(traceWave){
                if(trace_enable || (main_time % trace_sporadic_period) < trace_sporadic_trigger) tfp->dump(main_time);
                //将波形缓冲区文件的数据刷新到VCD文件中
                if(main_time % 100000 == 0) tfp->flush();
            }
            #endif

            //通过上次进度以来经过的时间，显示周期数与频率
            if(progressPeriod != 0.0 && main_time % 20000 == 0){
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now-progressLast).count();
                if(elapsed > progressPeriod*1000){
                    auto cycles = (main_time-progressMainTimeLast)/2;
                    auto hz = (u64)(cycles/(elapsed/1000));
                    progressMainTimeLast = main_time;
                    progressLast = now;
                    printf("[PROGRESS] %ld cycles %ld KHz\n", cycles, hz/1000);
                }
            }

            top->clk = !top->clk;
            //复位前5个周期
            top->reset = (main_time < 10) ? 1 : 0;
            #ifdef EMBEDDED_JTAG
                top->debug_reset = top->reset;
                if(top->EmbeddedJtagPlugin_logic_ndmreset) {
                    if(!top->reset){
                        for(SimElement* simElement : simElements) simElement->onReset();
                    }
                    top->reset = 1;
                }
            #endif
                //set start pc
            if(main_time < 11 && startPc != 0x80000000) top->NaxRiscv->PcPlugin_logic_fetchPc_pcReg = startPc;
            if(!top->clk){
                top->eval(); //更新状态
                if(Verilated::gotFinish()) failure();
            } else {
                //先执行precycle
                for(SimElement* simElement : simElements) if(!top->reset || simElement->withoutReset) simElement->preCycle();

                if(!top->reset) {
                    if(timeout_enabled && cycleSinceLastCommit == 10000){
                        printf("NO PROGRESS the cpu hasn't commited anything since too long\n");
                        failure();
                    }
                    cycleSinceLastCommit += 1;

                    for(int i = 0;i < COMMIT_COUNT;i++){
                        if(CHECK_BIT(topInternal->commit_mask, i)){
                            cycleSinceLastCommit = 0;
                            int robId = topInternal->commit_robId + i;
                            auto &robCtx = whitebox->robCtx[robId];
                            robIdChecked = robId;
                            commits += 1;
                            traps_since_commit = 0;
        //                        printf("Commit %d %x\n", robId, whitebox->robCtx[robId].pc);
                            RvData pc = robCtx.pc;
                            //是否进行spike检查
                            if(!spike_enabled){
                                last_commit_pc = pc;
                                if(pcToEvent.count(pc) != 0){
                                    for(auto event : pcToEvent[pc]){
                                        event(pc);
                                    }
                                }
                            }
                            if(spike_enabled) {
                                RvData spike_pc = state->pc;
                                spikeStep(robCtx);
                                last_commit_pc = pc;
                                //与spike模拟的pc不同时会报错
                                assertEq("MISSMATCH PC", pc, spike_pc);
                                for (auto item : state->log_reg_write) {
                                    if (item.first == 0)
                                      continue;

                                    int rd = item.first >> 4;
                                    switch (item.first & 0xf) {
                                    case 0: { //integer
                                        assertTrue("INTEGER WRITE MISSING", whitebox->robCtx[robId].integerWriteValid);
                                        assertEq("INTEGER WRITE DATA", whitebox->robCtx[robId].integerWriteData, item.second.v[0]);
                                    } break;
                                    case 1: { //float
                                        //TODO FPU track float writes
                                        assertTrue("FLOAT WRITE MISSING", whitebox->robCtx[robId].floatWriteValid);
                                        if(whitebox->robCtx[robId].floatWriteData != (RvFloat)item.second.v[0]){
                                            printf("\n*** FLOAT WRITE DATA DUT=%lx REF=%lx ***\n\n", (u64)whitebox->robCtx[robId].floatWriteData, (u64) item.second.v[0]);\
                                            failure();
                                        }
                                    } break;
                                    case 4:{ //CSR
                                        u64 inst = state->last_inst.bits();
                                        switch(inst){
                                        case 0x30200073: //MRET
                                        case 0x10200073: //SRET
                                        case 0x00200073: //URET
                                            break;
                                        default:
                                            if(inst & 0x7F == 0x73 && inst & 0x3000 != 0){
                                                assertTrue("CSR WRITE MISSING", whitebox->robCtx[robId].csrWriteDone);
                                                assertEq("CSR WRITE ADDRESS", whitebox->robCtx[robId].csrAddress & 0xCFF, rd & 0xCFF);
//                                                assertEq("CSR WRITE DATA", whitebox->robCtx[robId].csrWriteData, item.second.v[0]);
                                            }
                                            break;
                                        }
                                    } break;
                                    default: {
                                        printf("??? unknown spike trace %lx\n", item.first & 0xf);
                                        failure();
                                    } break;
                                    }
                                }

                                #ifdef RVF
//                                if(state->fpu_flags_set){
//                                    printf("Miaou %lx %x\n", pc, state->fpu_flags_set);
//                                }
                                assertEq("FPU FLAG MISSMATCH", whitebox->robCtx[robId].floatFlags, state->fpu_flags_set);
                                state->fpu_flags_set = 0;
                                #endif
                            }

                            if(pcToEvent.count(pc) != 0){
                                for(auto event : pcToEvent[pc]){
                                    event(pc);
                                }
                            }

                            if(pc == statsToggleAt) {
                                whitebox->statsCaptureEnable = !whitebox->statsCaptureEnable;
                                //cout << "Stats capture " << whitebox->statsCaptureEnable << " at " << main_time << endl;
                            }
                            if(pc == statsStartAt) whitebox->statsCaptureEnable = true;
                            if(pc == statsStopAt) whitebox->statsCaptureEnable = false;
                            whitebox->robCtx[robId].clear();
                        }
                    }

                    if(spike_enabled) spikeSyncTrap();
                }
                top->eval();
                if(Verilated::gotFinish()) failure();
                for(SimElement* simElement : simElements) if(!top->reset || simElement->withoutReset) simElement->postCycle();
            }
        }
    }catch (const successException e) {
        printf("SUCCESS %s\n", simName.c_str());
        remove((outputDir + "/FAIL").c_str());
        auto f = fopen((outputDir + "/PASS").c_str(),"w");
        fclose(f);
        if(simMaster) simMasterWriteHeader(SIM_MS_PASS);
    } catch (const std::exception& e) {
        ++main_time;
        #ifdef TRACE
        if(traceWave){
        tfp->dump(main_time);
        if(main_time % 100000 == 0) tfp->flush();
        }
        #endif
        printf("TIME=%ld\n", main_time);
        printf("LAST PC COMMIT=%lx\n", last_commit_pc);
        printf("INCOMING SPIKE PC=%lx\n", state->pc);
        printf("ROB_ID=x%x\n", robIdChecked);
        printf("FAILURE %s\n", simName.c_str());
        remove((outputDir + "/PASS").c_str());
        auto f = fopen((outputDir + "/FAIL").c_str(),"w");
        fclose(f);
        if(simMaster) {
            simMasterMainTime();
            simMasterWriteHeader(SIM_MS_FAIL);
        }
    }
    passFailWritten = true;

    if(statsPrint){
        printf("STATS :\n%s", whitebox->stats.report("  ", statsPrintHist).c_str());
    }
}

void cleanup(){
    if(fptr) {
        fflush(fptr);
        fclose(fptr);
    }

    #ifdef TRACE
    if(traceWave){
        tfp->flush();
        tfp->close();
    }
    #endif
    top->final();

    delete top;
    if(proc) delete proc;
    top = NULL;
    exit(0);
}

int main(int argc, char** argv, char** env){
    /*
     * 特定的信号并进行处理,SIGSEGV表示访问无效的内存地址
     * remove()删除文件
     * cleanup : 刷新缓冲流，关闭文件的指针
     * parseArgFirst ： 这个函数用于解析命令行传入的option，生成输出目录,并设置仿真时的配置参数
     * verilatorInit ：设置Verilator的全局变量信息，并创建log文件夹记录仿真信息
     * rtlInit(); 初始化naxiriscv
     *
     * parseArgsSecond：往内存中装载对应的文件，解析相关的符号，还可以设置访存的时延
     * simMasterSlaveInit ： 提供了网络通信的相关实现
    */
     signal(SIGSEGV, handler_crash);

    try {
        parseArgFirst(argc, argv);
        verilatorInit(argc, argv);
        //why use spike,test Excute the instruction，创建spikeInit处理器实例
        spikeInit();
        //Init the Naxriscv CPU
        rtlInit();

        //deal the command again
        parseArgsSecond(argc, argv);

        //tcp-server-client-implementation
        simMasterSlaveInit();

        //start simulation
        simLoop();
    } catch (const std::exception& e) {
        if(!passFailWritten){
            printf("FAILURE %s\n", simName.c_str());
            remove((outputDir + "/PASS").c_str());
            auto f = fopen((outputDir + "/FAIL").c_str(),"w");
            fclose(f);
        }
    }
    cleanup();
    return 0;
}
