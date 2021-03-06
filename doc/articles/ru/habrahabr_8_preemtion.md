��� ������ �� ����� ������ ��� <a href="https://habrahabr.ru/post/282037/">����������, � ������� ����������� �������� ��������� ������������ ���������� � ������������� ��</a>.

����� � ��������, ��� ������������� ��������������� ������������ �� <s>����������</s> ������������.

���� ����� ����������� ������. � ������ ���� ������, ������ ���������� ����������, ���������� ���������������� ��� ���� � ������ ��������� � ���� ��������� ���������������. ���� ��� ������ ������������ ����������� ��������� �� ����� ����, ��� � ������� � <a href="https://habrahabr.ru/post/282037/">���������� ������</a>.

��, ��� ������, ���� ������. ��. <a href="https://github.com/dzavalishin/phantomuserland/blob/master/oldtree/kernel/phantom/i386/interrupts.c">��� ��� ������</a>.

��� "�����" ���������� �������� ��� � ������ �������� ����������� ����������, ������ - �� �������, ��� � � ������ "�����������" ���������� - �������, ����������, ����� �� ����������, �� ��������� ����������� ����������� ����������. ����� ������ ������������ ��������� �����, ���� �� (��������, � ������ ��������� �������������) ���� ������������� ���� � �� ����� �����, ���� �������� ��������� ����������.

��-������, ����� ��� ��� ���������� ������� ���������� � ������ �����, ����� ��������� ���� ����������. ���������/���������� ���������� "�����", ��� ���������� ��������, � ��� ����� "���������", �������, ��� �� ��������� ������������, �� ����, ��� �� ������������� � ������ ����. ����� ��������� ����������� ���������� ����� ��������� ����� �������� � ������� ���������� ������������� ���������������. �� � ��� ������� ���������� �� ����� ��������, ���� ����� ��� ����������� ���� ������ ���������� ������ ����.

������� ������� �������� ���������� ����������. ����� ��������, ��� ��� �� ���� ��������� - ��� �� �� �������� ������������ ������� ����������,  ������� ���� ������ �� �����������.

� ��� ������, ����� �� �� ��� ��������� ������ ������� ������������ ����������, �� ���������� ���������� ��������� � ������� ������� ��������� ���������� ���������� �� ���� (�, �������, ���� ���������� ��������� ���������), �� ����� ���������, ��� �� ������� soft irq, � ���� �� - ��������� ���.

��� ���� ��������, ��� � ������� ����� ���������� irq_nest ����� ���� ������� ������������ ��������� ���������� � ���� <b>����������</b> ������� ����������� ����������. �� ���� ���� ��� ����� ����, ��, ������������, ����� ���� ����������� �������� ���������� ����������, ��� ������� �� ���������� ���������� � <b>����</b> ������ �� ���������� ����������.

<source lang="cpp">
    if(irq_nest)
        return;

    // Now for soft IRQs
    irq_nest = SOFT_IRQ_DISABLED|SOFT_IRQ_NOT_PENDING;
    hal_softirq_dispatcher(ts);
    ENABLE_SOFT_IRQ();
</source>

�����������, ��� ���� �� ����������� ���������� ����������, ���� ���������� ���������� ���������. �� ��������� ������������ �� �� ����� ��������. �� - ��. ���������� ������ - ���� �������� ���� ����� ����� � ����������, �� �� ����� �������� ���������� ���������� - ����� ����, �� ������� �� ������������, ������� �� ������������ �������. ���� ��� ����� ����� ��������� ���������� ����������. �������, � ��� �� �������.

��� ������������� ����� ���� ������������ ������� ����������� ����������

<source lang="cpp">
    hal_set_softirq_handler( SOFT_IRQ_THREADS, (void *)phantom_scheduler_soft_interrupt, 0 );
</source>

���� �������, ���� �� ������� ������ ��������, �������� � ������ phantom_thread_switch(), �� ���� ������ �������� � ������������ �� ��������� ����.

�������� ��� �������. ������ - ��� ���� "������" ���������. ��������, ����� �� �������� ��������� ��� �������� ������� - ���� ���� ����������.

��� ����� �� ������� ������ ����������� ���������� � ����������� (�����, �� ����� - ����� ������������) ���������� ����������.

<source lang="cpp">
void
phantom_scheduler_request_soft_irq()
{
    hal_request_softirq(SOFT_IRQ_THREADS);
    __asm __volatile("int $15");
}
</source>

��� ������� ����, ��� ������� � ����, ��� ������� phantom_thread_switch ����� ������� �� ��������� ����������� ����������.

������: � ��� �������� ���������� ����������, ����� ��������� ���������� � ��������� ������ ���� �� ��������� ����������� �� ����� ������� ����������?

��� ����� ���� ��� ����� ������:

<source lang="cpp">
void
phantom_scheduler_schedule_soft_irq()
{
    hal_request_softirq(SOFT_IRQ_THREADS);
}
</source>

�� ����������� ��� �����. ������ ���������� ���������� ���������� ����������� �������:

<source lang="cpp">
// Called from timer interrupt 100 times per sec.
void phantom_scheduler_time_interrupt(void)
{
    if(GET_CURRENT_THREAD()->priority & THREAD_PRIO_MOD_REALTIME)
        return; // Realtime thread will run until it blocks or reschedule requested

    if( (GET_CURRENT_THREAD()->ticks_left--) <= 0 )
        phantom_scheduler_request_reschedule();
}
</source>

��� �������� ������, ��� �������������� ���������� ���� ticks_left, � ���� ��������� �� ���� - ����������� ������������ ����.

���� ���������� ticks_left ���������� �������, ����� �������� ���� ��� ������� - �� ����������� � ��� ���������� ����� 10 ���� ����������, ������� ���� ���������� (���� �� ���� �� ������� ������������ ����).

����� ������ ������� ����� ���������� ������������� (���������� ���������� ����� ������� ���������� ���� �� ���������) ��� ��������� ��������� (����� ����� ������������������ ����� ����� ������� ���������).

� ����� ���� ��������, ��� ������� phantom_scheduler_request_reschedule() ����� ������, ��� ����, ��� ������� ���� ������������, ���� ������ ������ �� ���������.

��� ������, ����� ����� ���� �������, ���� ������� ���� �������������� �������� �������������, �� ������� ���� ������������� ���� � ������� (��� ����� - realtime) �����������. 

��� �� ���� ���� ����� ������ ���������� ���� - �������� ������������ ����� ������ � ����� ������������ ����������, ��� �������� ����.

��� ������� ������� ���������� ��������� �������� ���� (struct phantom_thread) � �������.

���� cpu �������� ����������� ��� ������ ����������� ����, � ������� ����������� ��������� ���������� ��� ��������� ����. cpu_id - ����� ����������, �� ������� ���� ����������� � ��������� ��� ��� �������� ������. tid - ������ ������������� ����. owner ����������� ��������� ������ �������, ����� ��������� ���� ������, ����������� ���� �� ���������� ������. ���� ���� ����������� ���������� ������������� � ������� - pid ������ ����� �������� ������, � �������� ���� �����������. ��� - ������������� ��� �������.

<source lang="cpp">
    /** NB! Exactly first! Accessed from asm. */
    cpu_state_save_t            cpu;

    //! on which CPU this thread is dispatched now
    int                         cpu_id; 

    int                         tid;

    //! phantom thread ref, etc
    void *                      owner;

    //! if this thread runs Unix simulation process - here is it
    pid_t                       pid;

    const char *                name;
</source>

ctty - ����� stdin ��� ����, ����������� ��� ����� � ����������� �����������. stack/kstack - ����������� � ���������� ����� �������� �����, �������������� ��� user � kernel mode. start_func � start_func_arg - ����� ����� � ������� ("main") ���� � �������� ���� �������.

<source lang="cpp">
    wtty_t *                    ctty; 

    void *                      stack;
    physaddr_t             stack_pa;
    size_t                      stack_size;

    void *                      kstack;
    physaddr_t              kstack_pa;
    size_t                      kstack_size;
    void *                      kstack_top; // What to load to ESP

    void *                      start_func_arg;
    void                        (*start_func)(void *);
</source>

sleep_flags - �������� ��������� ���� �� ��� ��� ���� �������. ���� �� ���� - ���� ��������� ������ (��� ��������, �������, �� ��������, ������ � �.�.). thread_flags - ��������� �������� ����: ���� ����������� ����������� ������ ������, � ���� �������� ������� ��������� ������������� � �.�.

waitcond/mutex/sem - ���� ���� �� ���� ���������, ��� ��� ������������. ownmutex - ��� ���� ������� ���� mutex, ���� ����� - ���� ����������. (��� �������� ��, ���, ����������.)

sleep_event - ����������� ���� �������� ������������� ������ � ��������� - ��������� ���������� ���� ������ ����� ��������� ���������� �������.

chain - ����������� ��� ���������� ���� � ������� ���� ������ ��������� ������������� ��� ��������� �����.

kill_chain - ������� �� ������. ����������� ��������� ���� ���������� ���������� ���������������� ������ ����� (������������ ������, ��������� ��������� � ��), � ��� - ������� � ���.

<source lang="cpp">
    u_int32_t                   thread_flags; // THREAD_FLAG_xxx

    /** if this field is zero, thread is ok to run. */
    u_int32_t                   sleep_flags; //THREAD_SLEEP_xxx

    hal_cond_t *                waitcond;
    hal_mutex_t *               waitmutex;
    hal_mutex_t *               ownmutex;
    hal_sem_t *                 waitsem;

    queue_chain_t               chain; // used by mutex/cond code to chain waiting threads
    queue_chain_t               kill_chain; // used kill code to chain threads to kill

    //* Used to wake with timer, see hal_sleep_msec
    timedcall_t                 sleep_event; 
</source>

snap_lock - ���� ��������� � ���������, � ������� ������ ������ snapshot.

preemption_disabled - ���� ������ ������� � ����������. ������-�� ������ � ���� ����� ����� ���, �������� � SMP �����.

death_handler - ����� ������, ���� ���� �������. atexit.

trap_handler - ��� ������ ����, ��� � user mode ���������� ������� - ������� ����������, ���� ���� ������� � �������� ����������.

<source lang="cpp">
    int                         snap_lock; // nonzero = can't begin a snapshot
    int                         preemption_disabled;

    //! void (*handler)( phantom_thread_t * )
    void *                      death_handler; // func to call if thread is killed

    //! Func to call on trap (a la unix signals), returns nonzero if can't handle
    int 			(*trap_handler)( int sig_no, struct trap_state *ts );
</source>

��������� - ��������� ��������. ��� �� ������: 

priority �������� ��������� ���� (������ � ������� - realtime, normal, idle)

ticks_left - ������� "�����" (10 ���� ����������) ���� ���������� �� ����������

runq_chain - ���� ���� ������ � ����������, �� �� �����������, �� ��� ������������ � ������� �� ����������.

sw_unlock - �������� ��������� �� �������, ������� ����� ������ ����� ������ ���� � ����������, ������������ � ���������� ���������� �������������.

<source lang="cpp">
    u_int32_t                   priority;

    /**
     * How many (100HZ) ticks this thread can be on CPU before resched.
     * NB! Signed, so that underrun is not a problem.
    **/
    int32_t                   	ticks_left;

    /** Used by runq only. Is not 0 if on runq. */
    queue_chain_t		runq_chain;

    /** Will be unlocked just after this thread is switched off CPU */
    hal_spinlock_t              *sw_unlock;
</source>


��������� ������, ������� ���� �� �������� � ������� ����: � ������� ������ ���� ���� (��������� �����, �� ����� �����������), ������� �������� �� ���������, ���� ������� �� ����� �� ����� ��������� ������ ����.

��� ���� �� ������ ������ - ��� ��������� ���������� ����������, ������� ������������� ��������� �� ��������� ����������, � ������� ����� ���������� ����. ����� � ���������� � ������� ��������� �������� ������� �������� ����������, � ��������� ���������� �������� ������������� � �����.

��. ��������, �� ������� - ��. <a href="https://habrahabr.ru/post/282213/">����������� �����</a>.
