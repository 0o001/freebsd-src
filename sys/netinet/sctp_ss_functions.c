/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010-2012, by Michael Tuexen. All rights reserved.
 * Copyright (c) 2010-2012, by Randall Stewart. All rights reserved.
 * Copyright (c) 2010-2012, by Robin Seggelmann. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * a) Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * b) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <netinet/sctp_pcb.h>

/*
 * Default simple round-robin algorithm.
 * Just iterates the streams in the order they appear.
 */

static void
sctp_ss_default_add(struct sctp_tcb *, struct sctp_association *,
    struct sctp_stream_out *,
    struct sctp_stream_queue_pending *);

static void
sctp_ss_default_remove(struct sctp_tcb *, struct sctp_association *,
    struct sctp_stream_out *,
    struct sctp_stream_queue_pending *);

static void
sctp_ss_default_init(struct sctp_tcb *stcb, struct sctp_association *asoc)
{
	uint16_t i;

	SCTP_TCB_LOCK_ASSERT(stcb);

	asoc->ss_data.locked_on_sending = NULL;
	asoc->ss_data.last_out_stream = NULL;
	TAILQ_INIT(&asoc->ss_data.out.wheel);
	/*
	 * If there is data in the stream queues already, the scheduler of
	 * an existing association has been changed. We need to add all
	 * stream queues to the wheel.
	 */
	for (i = 0; i < asoc->streamoutcnt; i++) {
		stcb->asoc.ss_functions.sctp_ss_add_to_stream(stcb, asoc,
		    &asoc->strmout[i],
		    NULL);
	}
	return;
}

static void
sctp_ss_default_clear(struct sctp_tcb *stcb, struct sctp_association *asoc,
    bool clear_values SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	while (!TAILQ_EMPTY(&asoc->ss_data.out.wheel)) {
		struct sctp_stream_out *strq;

		strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		KASSERT(strq->ss_params.scheduled, ("strq %p not scheduled", (void *)strq));
		TAILQ_REMOVE(&asoc->ss_data.out.wheel, strq, ss_params.ss.rr.next_spoke);
		strq->ss_params.scheduled = false;
	}
	asoc->ss_data.last_out_stream = NULL;
	return;
}

static void
sctp_ss_default_init_stream(struct sctp_tcb *stcb, struct sctp_stream_out *strq, struct sctp_stream_out *with_strq)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (with_strq != NULL) {
		if (stcb->asoc.ss_data.locked_on_sending == with_strq) {
			stcb->asoc.ss_data.locked_on_sending = strq;
		}
		if (stcb->asoc.ss_data.last_out_stream == with_strq) {
			stcb->asoc.ss_data.last_out_stream = strq;
		}
	}
	strq->ss_params.scheduled = false;
	return;
}

static void
sctp_ss_default_add(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq,
    struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/* Add to wheel if not already on it and stream queue not empty */
	if (!TAILQ_EMPTY(&strq->outqueue) && !strq->ss_params.scheduled) {
		TAILQ_INSERT_TAIL(&asoc->ss_data.out.wheel,
		    strq, ss_params.ss.rr.next_spoke);
		strq->ss_params.scheduled = true;
	}
	return;
}

static bool
sctp_ss_default_is_empty(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_association *asoc)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	return (TAILQ_EMPTY(&asoc->ss_data.out.wheel));
}

static void
sctp_ss_default_remove(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq,
    struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/*
	 * Remove from wheel if stream queue is empty and actually is on the
	 * wheel
	 */
	if (TAILQ_EMPTY(&strq->outqueue) && strq->ss_params.scheduled) {
		if (asoc->ss_data.last_out_stream == strq) {
			asoc->ss_data.last_out_stream = TAILQ_PREV(asoc->ss_data.last_out_stream,
			    sctpwheel_listhead,
			    ss_params.ss.rr.next_spoke);
			if (asoc->ss_data.last_out_stream == NULL) {
				asoc->ss_data.last_out_stream = TAILQ_LAST(&asoc->ss_data.out.wheel,
				    sctpwheel_listhead);
			}
			if (asoc->ss_data.last_out_stream == strq) {
				asoc->ss_data.last_out_stream = NULL;
			}
		}
		if (asoc->ss_data.locked_on_sending == strq) {
			asoc->ss_data.locked_on_sending = NULL;
		}
		TAILQ_REMOVE(&asoc->ss_data.out.wheel, strq, ss_params.ss.rr.next_spoke);
		strq->ss_params.scheduled = false;
	}
	return;
}

static struct sctp_stream_out *
sctp_ss_default_select(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net,
    struct sctp_association *asoc)
{
	struct sctp_stream_out *strq, *strqt;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (asoc->ss_data.locked_on_sending != NULL) {
		KASSERT(asoc->ss_data.locked_on_sending->ss_params.scheduled,
		    ("locked_on_sending %p not scheduled",
		    (void *)asoc->ss_data.locked_on_sending));
		return (asoc->ss_data.locked_on_sending);
	}
	strqt = asoc->ss_data.last_out_stream;
	KASSERT(strqt == NULL || strqt->ss_params.scheduled,
	    ("last_out_stream %p not scheduled", (void *)strqt));
default_again:
	/* Find the next stream to use */
	if (strqt == NULL) {
		strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
	} else {
		strq = TAILQ_NEXT(strqt, ss_params.ss.rr.next_spoke);
		if (strq == NULL) {
			strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		}
	}
	KASSERT(strq == NULL || strq->ss_params.scheduled,
	    ("strq %p not scheduled", (void *)strq));

	/*
	 * If CMT is off, we must validate that the stream in question has
	 * the first item pointed towards are network destination requested
	 * by the caller. Note that if we turn out to be locked to a stream
	 * (assigning TSN's then we must stop, since we cannot look for
	 * another stream with data to send to that destination). In CMT's
	 * case, by skipping this check, we will send one data packet
	 * towards the requested net.
	 */
	if (net != NULL && strq != NULL &&
	    SCTP_BASE_SYSCTL(sctp_cmt_on_off) == 0) {
		if (TAILQ_FIRST(&strq->outqueue) &&
		    TAILQ_FIRST(&strq->outqueue)->net != NULL &&
		    TAILQ_FIRST(&strq->outqueue)->net != net) {
			if (strq == asoc->ss_data.last_out_stream) {
				return (NULL);
			} else {
				strqt = strq;
				goto default_again;
			}
		}
	}
	return (strq);
}

static void
sctp_ss_default_scheduled(struct sctp_tcb *stcb,
    struct sctp_nets *net SCTP_UNUSED,
    struct sctp_association *asoc,
    struct sctp_stream_out *strq,
    int moved_how_much SCTP_UNUSED)
{
	struct sctp_stream_queue_pending *sp;

	KASSERT(strq != NULL, ("strq is NULL"));
	KASSERT(strq->ss_params.scheduled, ("strq %p is not scheduled", (void *)strq));
	SCTP_TCB_LOCK_ASSERT(stcb);

	asoc->ss_data.last_out_stream = strq;
	if (asoc->idata_supported == 0) {
		sp = TAILQ_FIRST(&strq->outqueue);
		if ((sp != NULL) && (sp->some_taken == 1)) {
			asoc->ss_data.locked_on_sending = strq;
		} else {
			asoc->ss_data.locked_on_sending = NULL;
		}
	} else {
		asoc->ss_data.locked_on_sending = NULL;
	}
	return;
}

static void
sctp_ss_default_packet_done(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net SCTP_UNUSED,
    struct sctp_association *asoc SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/* Nothing to be done here */
	return;
}

static int
sctp_ss_default_get_value(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_association *asoc SCTP_UNUSED,
    struct sctp_stream_out *strq SCTP_UNUSED, uint16_t *value SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/* Nothing to be done here */
	return (-1);
}

static int
sctp_ss_default_set_value(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_association *asoc SCTP_UNUSED,
    struct sctp_stream_out *strq SCTP_UNUSED, uint16_t value SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/* Nothing to be done here */
	return (-1);
}

static bool
sctp_ss_default_is_user_msgs_incomplete(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_association *asoc)
{
	struct sctp_stream_out *strq;
	struct sctp_stream_queue_pending *sp;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (asoc->stream_queue_cnt != 1) {
		return (false);
	}
	strq = asoc->ss_data.locked_on_sending;
	if (strq == NULL) {
		return (false);
	}
	sp = TAILQ_FIRST(&strq->outqueue);
	if (sp == NULL) {
		return (false);
	}
	return (sp->msg_is_complete == 0);
}

/*
 * Real round-robin algorithm.
 * Always iterates the streams in ascending order.
 */
static void
sctp_ss_rr_add(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq,
    struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	struct sctp_stream_out *strqt;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (!TAILQ_EMPTY(&strq->outqueue) && !strq->ss_params.scheduled) {
		if (TAILQ_EMPTY(&asoc->ss_data.out.wheel)) {
			TAILQ_INSERT_HEAD(&asoc->ss_data.out.wheel, strq, ss_params.ss.rr.next_spoke);
		} else {
			strqt = TAILQ_FIRST(&asoc->ss_data.out.wheel);
			while (strqt != NULL && (strqt->sid < strq->sid)) {
				strqt = TAILQ_NEXT(strqt, ss_params.ss.rr.next_spoke);
			}
			if (strqt != NULL) {
				TAILQ_INSERT_BEFORE(strqt, strq, ss_params.ss.rr.next_spoke);
			} else {
				TAILQ_INSERT_TAIL(&asoc->ss_data.out.wheel, strq, ss_params.ss.rr.next_spoke);
			}
		}
		strq->ss_params.scheduled = true;
	}
	return;
}

/*
 * Real round-robin per packet algorithm.
 * Always iterates the streams in ascending order and
 * only fills messages of the same stream in a packet.
 */
static struct sctp_stream_out *
sctp_ss_rrp_select(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net SCTP_UNUSED,
    struct sctp_association *asoc)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	return (asoc->ss_data.last_out_stream);
}

static void
sctp_ss_rrp_packet_done(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net,
    struct sctp_association *asoc)
{
	struct sctp_stream_out *strq, *strqt;

	SCTP_TCB_LOCK_ASSERT(stcb);

	strqt = asoc->ss_data.last_out_stream;
	KASSERT(strqt == NULL || strqt->ss_params.scheduled,
	    ("last_out_stream %p not scheduled", (void *)strqt));
rrp_again:
	/* Find the next stream to use */
	if (strqt == NULL) {
		strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
	} else {
		strq = TAILQ_NEXT(strqt, ss_params.ss.rr.next_spoke);
		if (strq == NULL) {
			strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		}
	}
	KASSERT(strq == NULL || strq->ss_params.scheduled,
	    ("strq %p not scheduled", (void *)strq));

	/*
	 * If CMT is off, we must validate that the stream in question has
	 * the first item pointed towards are network destination requested
	 * by the caller. Note that if we turn out to be locked to a stream
	 * (assigning TSN's then we must stop, since we cannot look for
	 * another stream with data to send to that destination). In CMT's
	 * case, by skipping this check, we will send one data packet
	 * towards the requested net.
	 */
	if (net != NULL && strq != NULL &&
	    SCTP_BASE_SYSCTL(sctp_cmt_on_off) == 0) {
		if (TAILQ_FIRST(&strq->outqueue) &&
		    TAILQ_FIRST(&strq->outqueue)->net != NULL &&
		    TAILQ_FIRST(&strq->outqueue)->net != net) {
			if (strq == asoc->ss_data.last_out_stream) {
				strq = NULL;
			} else {
				strqt = strq;
				goto rrp_again;
			}
		}
	}
	asoc->ss_data.last_out_stream = strq;
	return;
}

/*
 * Priority algorithm.
 * Always prefers streams based on their priority id.
 */
static void
sctp_ss_prio_clear(struct sctp_tcb *stcb, struct sctp_association *asoc,
    bool clear_values)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	while (!TAILQ_EMPTY(&asoc->ss_data.out.wheel)) {
		struct sctp_stream_out *strq;

		strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		KASSERT(strq->ss_params.scheduled, ("strq %p not scheduled", (void *)strq));
		if (clear_values) {
			strq->ss_params.ss.prio.priority = 0;
		}
		TAILQ_REMOVE(&asoc->ss_data.out.wheel, strq, ss_params.ss.prio.next_spoke);
		strq->ss_params.scheduled = false;
	}
	asoc->ss_data.last_out_stream = NULL;
	return;
}

static void
sctp_ss_prio_init_stream(struct sctp_tcb *stcb, struct sctp_stream_out *strq, struct sctp_stream_out *with_strq)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (with_strq != NULL) {
		if (stcb->asoc.ss_data.locked_on_sending == with_strq) {
			stcb->asoc.ss_data.locked_on_sending = strq;
		}
		if (stcb->asoc.ss_data.last_out_stream == with_strq) {
			stcb->asoc.ss_data.last_out_stream = strq;
		}
	}
	strq->ss_params.scheduled = false;
	if (with_strq != NULL) {
		strq->ss_params.ss.prio.priority = with_strq->ss_params.ss.prio.priority;
	} else {
		strq->ss_params.ss.prio.priority = 0;
	}
	return;
}

static void
sctp_ss_prio_add(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq, struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	struct sctp_stream_out *strqt;

	SCTP_TCB_LOCK_ASSERT(stcb);

	/* Add to wheel if not already on it and stream queue not empty */
	if (!TAILQ_EMPTY(&strq->outqueue) && !strq->ss_params.scheduled) {
		if (TAILQ_EMPTY(&asoc->ss_data.out.wheel)) {
			TAILQ_INSERT_HEAD(&asoc->ss_data.out.wheel, strq, ss_params.ss.prio.next_spoke);
		} else {
			strqt = TAILQ_FIRST(&asoc->ss_data.out.wheel);
			while (strqt != NULL && strqt->ss_params.ss.prio.priority < strq->ss_params.ss.prio.priority) {
				strqt = TAILQ_NEXT(strqt, ss_params.ss.prio.next_spoke);
			}
			if (strqt != NULL) {
				TAILQ_INSERT_BEFORE(strqt, strq, ss_params.ss.prio.next_spoke);
			} else {
				TAILQ_INSERT_TAIL(&asoc->ss_data.out.wheel, strq, ss_params.ss.prio.next_spoke);
			}
		}
		strq->ss_params.scheduled = true;
	}
	return;
}

static void
sctp_ss_prio_remove(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq, struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/*
	 * Remove from wheel if stream queue is empty and actually is on the
	 * wheel
	 */
	if (TAILQ_EMPTY(&strq->outqueue) && strq->ss_params.scheduled) {
		if (asoc->ss_data.last_out_stream == strq) {
			asoc->ss_data.last_out_stream = TAILQ_PREV(asoc->ss_data.last_out_stream,
			    sctpwheel_listhead,
			    ss_params.ss.prio.next_spoke);
			if (asoc->ss_data.last_out_stream == NULL) {
				asoc->ss_data.last_out_stream = TAILQ_LAST(&asoc->ss_data.out.wheel,
				    sctpwheel_listhead);
			}
			if (asoc->ss_data.last_out_stream == strq) {
				asoc->ss_data.last_out_stream = NULL;
			}
		}
		if (asoc->ss_data.locked_on_sending == strq) {
			asoc->ss_data.locked_on_sending = NULL;
		}
		TAILQ_REMOVE(&asoc->ss_data.out.wheel, strq, ss_params.ss.prio.next_spoke);
		strq->ss_params.scheduled = false;
	}
	return;
}

static struct sctp_stream_out *
sctp_ss_prio_select(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net,
    struct sctp_association *asoc)
{
	struct sctp_stream_out *strq, *strqt, *strqn;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (asoc->ss_data.locked_on_sending != NULL) {
		KASSERT(asoc->ss_data.locked_on_sending->ss_params.scheduled,
		    ("locked_on_sending %p not scheduled",
		    (void *)asoc->ss_data.locked_on_sending));
		return (asoc->ss_data.locked_on_sending);
	}
	strqt = asoc->ss_data.last_out_stream;
	KASSERT(strqt == NULL || strqt->ss_params.scheduled,
	    ("last_out_stream %p not scheduled", (void *)strqt));
prio_again:
	/* Find the next stream to use */
	if (strqt == NULL) {
		strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
	} else {
		strqn = TAILQ_NEXT(strqt, ss_params.ss.prio.next_spoke);
		if (strqn != NULL &&
		    strqn->ss_params.ss.prio.priority == strqt->ss_params.ss.prio.priority) {
			strq = strqn;
		} else {
			strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		}
	}
	KASSERT(strq == NULL || strq->ss_params.scheduled,
	    ("strq %p not scheduled", (void *)strq));

	/*
	 * If CMT is off, we must validate that the stream in question has
	 * the first item pointed towards are network destination requested
	 * by the caller. Note that if we turn out to be locked to a stream
	 * (assigning TSN's then we must stop, since we cannot look for
	 * another stream with data to send to that destination). In CMT's
	 * case, by skipping this check, we will send one data packet
	 * towards the requested net.
	 */
	if (net != NULL && strq != NULL &&
	    SCTP_BASE_SYSCTL(sctp_cmt_on_off) == 0) {
		if (TAILQ_FIRST(&strq->outqueue) &&
		    TAILQ_FIRST(&strq->outqueue)->net != NULL &&
		    TAILQ_FIRST(&strq->outqueue)->net != net) {
			if (strq == asoc->ss_data.last_out_stream) {
				return (NULL);
			} else {
				strqt = strq;
				goto prio_again;
			}
		}
	}
	return (strq);
}

static int
sctp_ss_prio_get_value(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_association *asoc SCTP_UNUSED,
    struct sctp_stream_out *strq, uint16_t *value)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (strq == NULL) {
		return (-1);
	}
	*value = strq->ss_params.ss.prio.priority;
	return (1);
}

static int
sctp_ss_prio_set_value(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq, uint16_t value)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (strq == NULL) {
		return (-1);
	}
	strq->ss_params.ss.prio.priority = value;
	sctp_ss_prio_remove(stcb, asoc, strq, NULL);
	sctp_ss_prio_add(stcb, asoc, strq, NULL);
	return (1);
}

/*
 * Fair bandwidth algorithm.
 * Maintains an equal throughput per stream.
 */
static void
sctp_ss_fb_clear(struct sctp_tcb *stcb, struct sctp_association *asoc,
    bool clear_values)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	while (!TAILQ_EMPTY(&asoc->ss_data.out.wheel)) {
		struct sctp_stream_out *strq;

		strq = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		KASSERT(strq->ss_params.scheduled, ("strq %p not scheduled", (void *)strq));
		if (clear_values) {
			strq->ss_params.ss.fb.rounds = -1;
		}
		TAILQ_REMOVE(&asoc->ss_data.out.wheel, strq, ss_params.ss.fb.next_spoke);
		strq->ss_params.scheduled = false;
	}
	asoc->ss_data.last_out_stream = NULL;
	return;
}

static void
sctp_ss_fb_init_stream(struct sctp_tcb *stcb, struct sctp_stream_out *strq, struct sctp_stream_out *with_strq)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (with_strq != NULL) {
		if (stcb->asoc.ss_data.locked_on_sending == with_strq) {
			stcb->asoc.ss_data.locked_on_sending = strq;
		}
		if (stcb->asoc.ss_data.last_out_stream == with_strq) {
			stcb->asoc.ss_data.last_out_stream = strq;
		}
	}
	strq->ss_params.scheduled = false;
	if (with_strq != NULL) {
		strq->ss_params.ss.fb.rounds = with_strq->ss_params.ss.fb.rounds;
	} else {
		strq->ss_params.ss.fb.rounds = -1;
	}
	return;
}

static void
sctp_ss_fb_add(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq, struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (!TAILQ_EMPTY(&strq->outqueue) && !strq->ss_params.scheduled) {
		if (strq->ss_params.ss.fb.rounds < 0)
			strq->ss_params.ss.fb.rounds = TAILQ_FIRST(&strq->outqueue)->length;
		TAILQ_INSERT_TAIL(&asoc->ss_data.out.wheel, strq, ss_params.ss.fb.next_spoke);
		strq->ss_params.scheduled = true;
	}
	return;
}

static void
sctp_ss_fb_remove(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq, struct sctp_stream_queue_pending *sp SCTP_UNUSED)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	/*
	 * Remove from wheel if stream queue is empty and actually is on the
	 * wheel
	 */
	if (TAILQ_EMPTY(&strq->outqueue) && strq->ss_params.scheduled) {
		if (asoc->ss_data.last_out_stream == strq) {
			asoc->ss_data.last_out_stream = TAILQ_PREV(asoc->ss_data.last_out_stream,
			    sctpwheel_listhead,
			    ss_params.ss.fb.next_spoke);
			if (asoc->ss_data.last_out_stream == NULL) {
				asoc->ss_data.last_out_stream = TAILQ_LAST(&asoc->ss_data.out.wheel,
				    sctpwheel_listhead);
			}
			if (asoc->ss_data.last_out_stream == strq) {
				asoc->ss_data.last_out_stream = NULL;
			}
		}
		if (asoc->ss_data.locked_on_sending == strq) {
			asoc->ss_data.locked_on_sending = NULL;
		}
		TAILQ_REMOVE(&asoc->ss_data.out.wheel, strq, ss_params.ss.fb.next_spoke);
		strq->ss_params.scheduled = false;
	}
	return;
}

static struct sctp_stream_out *
sctp_ss_fb_select(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net,
    struct sctp_association *asoc)
{
	struct sctp_stream_out *strq = NULL, *strqt;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (asoc->ss_data.locked_on_sending != NULL) {
		KASSERT(asoc->ss_data.locked_on_sending->ss_params.scheduled,
		    ("locked_on_sending %p not scheduled",
		    (void *)asoc->ss_data.locked_on_sending));
		return (asoc->ss_data.locked_on_sending);
	}
	if (asoc->ss_data.last_out_stream == NULL ||
	    TAILQ_FIRST(&asoc->ss_data.out.wheel) == TAILQ_LAST(&asoc->ss_data.out.wheel, sctpwheel_listhead)) {
		strqt = TAILQ_FIRST(&asoc->ss_data.out.wheel);
	} else {
		strqt = TAILQ_NEXT(asoc->ss_data.last_out_stream, ss_params.ss.fb.next_spoke);
	}
	do {
		if ((strqt != NULL) &&
		    ((SCTP_BASE_SYSCTL(sctp_cmt_on_off) > 0) ||
		    (SCTP_BASE_SYSCTL(sctp_cmt_on_off) == 0 &&
		    (net == NULL || (TAILQ_FIRST(&strqt->outqueue) && TAILQ_FIRST(&strqt->outqueue)->net == NULL) ||
		    (net != NULL && TAILQ_FIRST(&strqt->outqueue) && TAILQ_FIRST(&strqt->outqueue)->net != NULL &&
		    TAILQ_FIRST(&strqt->outqueue)->net == net))))) {
			if ((strqt->ss_params.ss.fb.rounds >= 0) &&
			    ((strq == NULL) ||
			    (strqt->ss_params.ss.fb.rounds < strq->ss_params.ss.fb.rounds))) {
				strq = strqt;
			}
		}
		if (strqt != NULL) {
			strqt = TAILQ_NEXT(strqt, ss_params.ss.fb.next_spoke);
		} else {
			strqt = TAILQ_FIRST(&asoc->ss_data.out.wheel);
		}
	} while (strqt != strq);
	return (strq);
}

static void
sctp_ss_fb_scheduled(struct sctp_tcb *stcb, struct sctp_nets *net SCTP_UNUSED,
    struct sctp_association *asoc, struct sctp_stream_out *strq,
    int moved_how_much SCTP_UNUSED)
{
	struct sctp_stream_queue_pending *sp;
	struct sctp_stream_out *strqt;
	int subtract;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (asoc->idata_supported == 0) {
		sp = TAILQ_FIRST(&strq->outqueue);
		if ((sp != NULL) && (sp->some_taken == 1)) {
			asoc->ss_data.locked_on_sending = strq;
		} else {
			asoc->ss_data.locked_on_sending = NULL;
		}
	} else {
		asoc->ss_data.locked_on_sending = NULL;
	}
	subtract = strq->ss_params.ss.fb.rounds;
	TAILQ_FOREACH(strqt, &asoc->ss_data.out.wheel, ss_params.ss.fb.next_spoke) {
		strqt->ss_params.ss.fb.rounds -= subtract;
		if (strqt->ss_params.ss.fb.rounds < 0)
			strqt->ss_params.ss.fb.rounds = 0;
	}
	if (TAILQ_FIRST(&strq->outqueue)) {
		strq->ss_params.ss.fb.rounds = TAILQ_FIRST(&strq->outqueue)->length;
	} else {
		strq->ss_params.ss.fb.rounds = -1;
	}
	asoc->ss_data.last_out_stream = strq;
	return;
}

/*
 * First-come, first-serve algorithm.
 * Maintains the order provided by the application.
 */
static void
sctp_ss_fcfs_add(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq SCTP_UNUSED,
    struct sctp_stream_queue_pending *sp);

static void
sctp_ss_fcfs_init(struct sctp_tcb *stcb, struct sctp_association *asoc)
{
	uint32_t x, n = 0, add_more = 1;
	struct sctp_stream_queue_pending *sp;
	uint16_t i;

	SCTP_TCB_LOCK_ASSERT(stcb);

	TAILQ_INIT(&asoc->ss_data.out.list);
	/*
	 * If there is data in the stream queues already, the scheduler of
	 * an existing association has been changed. We can only cycle
	 * through the stream queues and add everything to the FCFS queue.
	 */
	while (add_more) {
		add_more = 0;
		for (i = 0; i < asoc->streamoutcnt; i++) {
			sp = TAILQ_FIRST(&asoc->strmout[i].outqueue);
			x = 0;
			/* Find n. message in current stream queue */
			while (sp != NULL && x < n) {
				sp = TAILQ_NEXT(sp, next);
				x++;
			}
			if (sp != NULL) {
				sctp_ss_fcfs_add(stcb, asoc, &asoc->strmout[i], sp);
				add_more = 1;
			}
		}
		n++;
	}
	return;
}

static void
sctp_ss_fcfs_clear(struct sctp_tcb *stcb, struct sctp_association *asoc,
    bool clear_values SCTP_UNUSED)
{
	struct sctp_stream_queue_pending *sp;

	SCTP_TCB_LOCK_ASSERT(stcb);

	while (!TAILQ_EMPTY(&asoc->ss_data.out.list)) {
		sp = TAILQ_FIRST(&asoc->ss_data.out.list);
		KASSERT(sp->scheduled, ("sp %p not scheduled", (void *)sp));
		TAILQ_REMOVE(&asoc->ss_data.out.list, sp, ss_next);
		sp->scheduled = false;
	}
	asoc->ss_data.last_out_stream = NULL;
	return;
}

static void
sctp_ss_fcfs_init_stream(struct sctp_tcb *stcb, struct sctp_stream_out *strq, struct sctp_stream_out *with_strq)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (with_strq != NULL) {
		if (stcb->asoc.ss_data.locked_on_sending == with_strq) {
			stcb->asoc.ss_data.locked_on_sending = strq;
		}
		if (stcb->asoc.ss_data.last_out_stream == with_strq) {
			stcb->asoc.ss_data.last_out_stream = strq;
		}
	}
	strq->ss_params.scheduled = false;
	return;
}

static void
sctp_ss_fcfs_add(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq SCTP_UNUSED, struct sctp_stream_queue_pending *sp)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (!sp->scheduled) {
		TAILQ_INSERT_TAIL(&asoc->ss_data.out.list, sp, ss_next);
		sp->scheduled = true;
	}
	return;
}

static bool
sctp_ss_fcfs_is_empty(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_association *asoc)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	return (TAILQ_EMPTY(&asoc->ss_data.out.list));
}

static void
sctp_ss_fcfs_remove(struct sctp_tcb *stcb, struct sctp_association *asoc,
    struct sctp_stream_out *strq SCTP_UNUSED, struct sctp_stream_queue_pending *sp)
{
	SCTP_TCB_LOCK_ASSERT(stcb);

	if (sp->scheduled) {
		TAILQ_REMOVE(&asoc->ss_data.out.list, sp, ss_next);
		sp->scheduled = false;
	}
	return;
}

static struct sctp_stream_out *
sctp_ss_fcfs_select(struct sctp_tcb *stcb SCTP_UNUSED, struct sctp_nets *net,
    struct sctp_association *asoc)
{
	struct sctp_stream_out *strq;
	struct sctp_stream_queue_pending *sp;

	SCTP_TCB_LOCK_ASSERT(stcb);

	if (asoc->ss_data.locked_on_sending) {
		return (asoc->ss_data.locked_on_sending);
	}
	sp = TAILQ_FIRST(&asoc->ss_data.out.list);
default_again:
	if (sp != NULL) {
		strq = &asoc->strmout[sp->sid];
	} else {
		strq = NULL;
	}

	/*
	 * If CMT is off, we must validate that the stream in question has
	 * the first item pointed towards are network destination requested
	 * by the caller. Note that if we turn out to be locked to a stream
	 * (assigning TSN's then we must stop, since we cannot look for
	 * another stream with data to send to that destination). In CMT's
	 * case, by skipping this check, we will send one data packet
	 * towards the requested net.
	 */
	if (net != NULL && strq != NULL &&
	    SCTP_BASE_SYSCTL(sctp_cmt_on_off) == 0) {
		if (TAILQ_FIRST(&strq->outqueue) &&
		    TAILQ_FIRST(&strq->outqueue)->net != NULL &&
		    TAILQ_FIRST(&strq->outqueue)->net != net) {
			sp = TAILQ_NEXT(sp, ss_next);
			goto default_again;
		}
	}
	return (strq);
}

static void
sctp_ss_fcfs_scheduled(struct sctp_tcb *stcb,
    struct sctp_nets *net SCTP_UNUSED,
    struct sctp_association *asoc,
    struct sctp_stream_out *strq,
    int moved_how_much SCTP_UNUSED)
{
	struct sctp_stream_queue_pending *sp;

	KASSERT(strq != NULL, ("strq is NULL"));
	asoc->ss_data.last_out_stream = strq;
	if (asoc->idata_supported == 0) {
		sp = TAILQ_FIRST(&strq->outqueue);
		if ((sp != NULL) && (sp->some_taken == 1)) {
			asoc->ss_data.locked_on_sending = strq;
		} else {
			asoc->ss_data.locked_on_sending = NULL;
		}
	} else {
		asoc->ss_data.locked_on_sending = NULL;
	}
	return;
}

const struct sctp_ss_functions sctp_ss_functions[] = {
/* SCTP_SS_DEFAULT */
	{
		.sctp_ss_init = sctp_ss_default_init,
		.sctp_ss_clear = sctp_ss_default_clear,
		.sctp_ss_init_stream = sctp_ss_default_init_stream,
		.sctp_ss_add_to_stream = sctp_ss_default_add,
		.sctp_ss_is_empty = sctp_ss_default_is_empty,
		.sctp_ss_remove_from_stream = sctp_ss_default_remove,
		.sctp_ss_select_stream = sctp_ss_default_select,
		.sctp_ss_scheduled = sctp_ss_default_scheduled,
		.sctp_ss_packet_done = sctp_ss_default_packet_done,
		.sctp_ss_get_value = sctp_ss_default_get_value,
		.sctp_ss_set_value = sctp_ss_default_set_value,
		.sctp_ss_is_user_msgs_incomplete = sctp_ss_default_is_user_msgs_incomplete
	},
/* SCTP_SS_RR */
	{
		.sctp_ss_init = sctp_ss_default_init,
		.sctp_ss_clear = sctp_ss_default_clear,
		.sctp_ss_init_stream = sctp_ss_default_init_stream,
		.sctp_ss_add_to_stream = sctp_ss_rr_add,
		.sctp_ss_is_empty = sctp_ss_default_is_empty,
		.sctp_ss_remove_from_stream = sctp_ss_default_remove,
		.sctp_ss_select_stream = sctp_ss_default_select,
		.sctp_ss_scheduled = sctp_ss_default_scheduled,
		.sctp_ss_packet_done = sctp_ss_default_packet_done,
		.sctp_ss_get_value = sctp_ss_default_get_value,
		.sctp_ss_set_value = sctp_ss_default_set_value,
		.sctp_ss_is_user_msgs_incomplete = sctp_ss_default_is_user_msgs_incomplete
	},
/* SCTP_SS_RR_PKT */
	{
		.sctp_ss_init = sctp_ss_default_init,
		.sctp_ss_clear = sctp_ss_default_clear,
		.sctp_ss_init_stream = sctp_ss_default_init_stream,
		.sctp_ss_add_to_stream = sctp_ss_rr_add,
		.sctp_ss_is_empty = sctp_ss_default_is_empty,
		.sctp_ss_remove_from_stream = sctp_ss_default_remove,
		.sctp_ss_select_stream = sctp_ss_rrp_select,
		.sctp_ss_scheduled = sctp_ss_default_scheduled,
		.sctp_ss_packet_done = sctp_ss_rrp_packet_done,
		.sctp_ss_get_value = sctp_ss_default_get_value,
		.sctp_ss_set_value = sctp_ss_default_set_value,
		.sctp_ss_is_user_msgs_incomplete = sctp_ss_default_is_user_msgs_incomplete
	},
/* SCTP_SS_PRIO */
	{
		.sctp_ss_init = sctp_ss_default_init,
		.sctp_ss_clear = sctp_ss_prio_clear,
		.sctp_ss_init_stream = sctp_ss_prio_init_stream,
		.sctp_ss_add_to_stream = sctp_ss_prio_add,
		.sctp_ss_is_empty = sctp_ss_default_is_empty,
		.sctp_ss_remove_from_stream = sctp_ss_prio_remove,
		.sctp_ss_select_stream = sctp_ss_prio_select,
		.sctp_ss_scheduled = sctp_ss_default_scheduled,
		.sctp_ss_packet_done = sctp_ss_default_packet_done,
		.sctp_ss_get_value = sctp_ss_prio_get_value,
		.sctp_ss_set_value = sctp_ss_prio_set_value,
		.sctp_ss_is_user_msgs_incomplete = sctp_ss_default_is_user_msgs_incomplete
	},
/* SCTP_SS_FB */
	{
		.sctp_ss_init = sctp_ss_default_init,
		.sctp_ss_clear = sctp_ss_fb_clear,
		.sctp_ss_init_stream = sctp_ss_fb_init_stream,
		.sctp_ss_add_to_stream = sctp_ss_fb_add,
		.sctp_ss_is_empty = sctp_ss_default_is_empty,
		.sctp_ss_remove_from_stream = sctp_ss_fb_remove,
		.sctp_ss_select_stream = sctp_ss_fb_select,
		.sctp_ss_scheduled = sctp_ss_fb_scheduled,
		.sctp_ss_packet_done = sctp_ss_default_packet_done,
		.sctp_ss_get_value = sctp_ss_default_get_value,
		.sctp_ss_set_value = sctp_ss_default_set_value,
		.sctp_ss_is_user_msgs_incomplete = sctp_ss_default_is_user_msgs_incomplete
	},
/* SCTP_SS_FCFS */
	{
		.sctp_ss_init = sctp_ss_fcfs_init,
		.sctp_ss_clear = sctp_ss_fcfs_clear,
		.sctp_ss_init_stream = sctp_ss_fcfs_init_stream,
		.sctp_ss_add_to_stream = sctp_ss_fcfs_add,
		.sctp_ss_is_empty = sctp_ss_fcfs_is_empty,
		.sctp_ss_remove_from_stream = sctp_ss_fcfs_remove,
		.sctp_ss_select_stream = sctp_ss_fcfs_select,
		.sctp_ss_scheduled = sctp_ss_fcfs_scheduled,
		.sctp_ss_packet_done = sctp_ss_default_packet_done,
		.sctp_ss_get_value = sctp_ss_default_get_value,
		.sctp_ss_set_value = sctp_ss_default_set_value,
		.sctp_ss_is_user_msgs_incomplete = sctp_ss_default_is_user_msgs_incomplete
	}
};
