#pragma once

#include <stdint.h>
#include "debug.h"

// BUG: Currently NO handling of millis() rollover -- it's
// assumed that this device will be used for hours at a time,
// and never be on for the 50 days needed for millis rollover.
// However, if this code is ever reused for a long timing 
// project, this will need to be fixed.


typedef uint8_t qindex_t;


template <payload_t> struct Message
{
    unsigned long eta;
    payload_t payload;
    qindex_t next;
    void* callback;
};


template <payload_t, uint8_t QUEUE_SIZE> class MessageQueue
{
private:
    Message <payload_t> queue[QUEUE_SIZE];
    qindex_t head = QUEUE_SIZE;


public:
    void MessageQueue()
    {
        init();
    }


    // Initialise the queue.  This will also clear all
    // pending messages.
    bool init()
    {
        qindex_t i;
        for (i = 0; i < QUEUE_SIZE; i++)
            queue[i].callback = NULL;
        head = QUEUE_SIZE;
        return true;
    }


    // Dispatch the message msg to function callback, at time
    // delta in the future.
    bool send(void* callback, payload_t payload, unsigned long delta)
    {
        // callback must point to a valid function
        if (callback == NULL)
        {
            DEBUG_PRINT("WARN: callback == NULL, message discarded\n");
            return false;
        }

        // Find an empty entry in queue.
        qindex_t i;
        for (i = 0; i < QUEUE_SIZE; i++)
        {
            if (queue[i].callback == NULL)
                break;
        }
        if (i == QUEUE_SIZE)
        {
            // The queue is full
            DEBUG_PRINT("WARN: queue full, message discarded\n");
            return false;
        }

        // Copy the supplied values to queue[i]
        queue[i].eta = millis() + delta;
        queue[i].payload_t = payload;
        queue[i].callback = callback;

        // Find where queue[i].eta falls in the queue order.
        // We want to find a queue_before so that
        // queue_before->eta <= queue[i].eta && queue_before->next->eta >= queue[i].eta

        // Check first for the special case of the queue being empty
        if (head == QUEUE_SIZE)
        {
            // Terminate the freshly-made queue, set the head to i, and return
            queue[i].next = QUEUE_SIZE;
            head = i;
            return true;
        }

        // The queue is not empty.  Skip over the list, finding where to
        // insert the new message.  Three options:
        // At the front (if head == NULL, equivalently search_i_prev == NULL)
        // In the middle (if queue[i].eta < search_i->eta), insert between search_i_prev and search_i
        // At the end (if the entire queue is iterated over without a result.)
        qindex_t search_i, search_i_prev;
        search_i_prev = QUEUE_SIZE;
        for (search_i = head; search_i->next != QUEUE_SIZE; search_i = search_i->next)
        {
            if (queue[i].eta < queue[search_i].eta)
            {
                // Insert (queue + i) immediately between search_i_prev and search_i.
                queue[i].next = search_i;
                
                if (search_i_prev == QUEUE_SIZE)
                    head = i;                       // Special case: the new message will go to the front of the queue.
                else
                    queue[search_i_prev].next = i;  // General case: the new message will go between search_i_prev and search_i.

                return true;
            }
            search_i_prev = search_i;
        }

        // Special case: the new message will go at the end of the queue.
        queue[i].next = QUEUE_SIZE;
        queue[search_i].next = i;

        return true;
    }


    // Dispatch a message, if possible.  Returns true
    // if a message was dispatched, else false.
    bool run()
    {
        // Check for any messages
        if (head == QUEUE_SIZE)
            return false;

        unsigned long time, skew;
        time = millis();

        // Is the earliest message not yet due?
        if (queue[head].eta > time)
            return false;

        // Dispatch the message
        skew = time - eta;
        (*(queue[head].callback))(queue[head].payload, skew);

        // Delete the message
        queue[head].callback = NULL;
        head = queue[head].next;

        return true;
    }


    qindex_t queue_length()
    {
        qindex_t n = 0, i;

        for (i = head; i != QUEUE_SIZE; i = queue[i].next)
            n++;

        return n;
    }


    void print_queue()
    {
        qindex_t i;

        if (head == QUEUE_SIZE)
        {
            DEBUG_PRINT("Queue empty\n");
            return;
        }

        for (i = head; i != QUEUE_SIZE; i = queue[i].next)
        {
            DEBUG_PRINTF("i: %3d  ", i);
            print_message(queue[i]);
            DEBUG_PRINT("\n");
        }
    }


    void print_message(const Message <payload_t>& msg)
    {
        DEBUG_PRINTF("Call: %8p  Payload: %8x  Eta: %6d  Next: %3d", msg.callback, msg.payload, msg.eta, msg.next);
    }
}

