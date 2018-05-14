#include "encoding.h"
#include "util.h"

void enable_timer_interrupts()
{
    set_csr(mie, MIP_MTIP | MIP_STIP);
    set_csr(mstatus, MSTATUS_MIE | MSTATUS_SIE);
    //    set_csr(mstatus, MSTATUS_MIE | MSTATUS_SIE); // Works
}

static volatile unsigned interrupt_count;
static volatile unsigned local;

int main(int argc, char** argv)
{
    write_csr(mie, 0);
    write_csr(sie, 0);
    write_csr(mip, 0);
    write_csr(sip, 0);
    write_csr(mideleg, 0);
    write_csr(medeleg, 0);

    interrupt_count = 0;
    local = 0;
    unsigned hartid = read_csr(mhartid);
    unsigned m_status = read_csr(mstatus);
    unsigned s_status = read_csr(sstatus);

    enable_timer_interrupts();

    MTIMECMP[0] = MTIME + 0x500;

    printf("mstatus = 0x%llx   - sstatus = 0x%llx   -   mideleg = 0x%llx   -   medeleg = 0x%llx \n", m_status, s_status, read_csr(mideleg), read_csr(medeleg));
    printf("time =%lld    - timecmp = %lld\n", MTIME, MTIMECMP[0]);

    while (local < 500000) {
        local++;
        if(local % 50 == 0)
            printf("Waiting ..  mtime = %lld \n", MTIME);
    }

    return 0;
}
