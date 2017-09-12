/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_COMMS_INTERFACE_
#define _HELICS_COMMS_INTERFACE_
#pragma once

#include "ActionMessage.h"
#include "common/BlockingQueue.hpp"
#include <functional>
#include <thread>
#include <atomic>

namespace helics {

/** implementation for the core that uses zmq messages to communicate*/
class CommsInterface {

public:
	/** default constructor*/
	CommsInterface() = default;
	CommsInterface(const std::string &localTarget, const std::string &brokerTarget);
	/** destructor*/
	virtual ~CommsInterface();

	void transmit(int route_id, const ActionMessage &cmd);
	void addRoute(int route_id, const std::string &routeInfo);
	bool connect();
	void disconnect();
	void setCallback(std::function<void(ActionMessage &&)> callback);
	void setMessageSize(int maxMessageSize, int maxQueueSize);
protected:
	//enumeration of the connection status flags for more immediate feedback from the processing threads
	enum class connection_status :int
	{
		error = -1,	//!< some error occurred on the connection
		startup = 0, //!< the connection is in startup mode
		connected = 1,	//!< we are connected
		terminated=2,	//!< the connection has been terminated

	};
	std::atomic<connection_status> rx_status{ connection_status::startup }; //!< the status of the receiver thread
	std::string localTarget_; //!< the identifier for the receive target
	std::string brokerTarget_;	//!< the identifier for the broker target
	int maxMessageSize_ = 16 * 1024; //!< the maximum message size for the queues (if needed)
	int maxMessageCount_ = 1024;  //!< the maximum number of message to buffer (if needed)
	std::function<void(ActionMessage &&)> ActionCallback; //!< the callback for what to do with a received message
	BlockingQueue2<std::pair<int, ActionMessage>> txQueue; //!< set of messages waiting to be transmitted
	std::atomic<connection_status> tx_status{ connection_status::startup }; //!< the status of the transmitter thread
private:
	std::thread queue_transmitter; //!< single thread for sending data
	std::thread queue_watcher; //!< thread monitoring the receive queue
	virtual void queue_rx_function()=0;	//!< the functional loop for the receive queue
	virtual void queue_tx_function()=0;  //!< the loop for transmitting data
	virtual void closeTransmitter() = 0; //!< function to instruct the transmitter loop to close
	virtual void closeReceiver() = 0;  //!< function to instruct the receiver loop to close
};


} // namespace helics

#endif /* _HELICS_COMMS_INTERFACE_ */