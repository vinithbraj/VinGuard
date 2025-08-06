/*
 * Copyright (c) 2025 Vinith Raj. All rights reserved.
 *
 * Permission is hereby granted to view and read this source code
 * for educational or informational purposes only.
 *
 * Redistribution, modification, reproduction, or use in any software—whether
 * commercial, open-source, or personal—is strictly prohibited without prior
 * written permission from the author.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 *
 * Contact: vinithbraj(at)gmail(dot)com
 */


#pragma once

extern "C"
{
	#include <fltKernel.h>
	#pragma comment(lib, "fltmgr.lib")
}

#include "VinGuardKDebugMacros.h"
#include "VinGuardKMessageDefs.h"

	namespace VinGuard
	{
		const wchar_t port_name[] = L"\\c7fe3b44-4759-4a07-a37a-ef89a9316096";
		const int max_connections = 1;
		class kernel_user_com {

		public:

			PFLT_PORT m_server_port;
			PFLT_PORT m_client_port;
			PFLT_FILTER m_filter_handle;

			kernel_user_com():
				m_filter_handle(nullptr),
				m_server_port(nullptr),
				m_client_port(nullptr)
			{
			}

			NTSTATUS initialize(PFLT_FILTER filter_handle) {

				di("Enter coms registration");

				if (!filter_handle) {
					de("Invalid filter handle");
					return STATUS_INVALID_PARAMETER;
				}

				m_filter_handle = filter_handle;

				OBJECT_ATTRIBUTES port_attributes = { NULL };
				const ULONG flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
				UNICODE_STRING l_port_name = RTL_CONSTANT_STRING(port_name);

				// Todo: Restrict this
				PSECURITY_DESCRIPTOR sd;
				NTSTATUS status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);
				if (!NT_SUCCESS(status)) {
					return status;
				}

				InitializeObjectAttributes(
					&port_attributes,
					&l_port_name,
					flags, 0, sd);

				NTSTATUS result = FltCreateCommunicationPort(
					m_filter_handle,
					&m_server_port,
					&port_attributes,
					(PVOID)this,
					kernel_user_com::callback_on_connect,
					kernel_user_com::callback_on_disconnect,
					kernel_user_com::callback_on_message,
					max_connections);

				if (STATUS_SUCCESS != result) {
					de("Failed creating communication port - NTSTATUS = %ul", result);
				}

				di("Exit coms registration");
				return result;
			}

			void shutdown() {

				di("enter - coms shutdown");

				// Close the client port if it's open
				if (m_client_port) {
					FltCloseClientPort(m_filter_handle, &m_client_port);
					m_client_port = nullptr;
				}

				// Close the server port
				if (m_server_port) {
					FltCloseCommunicationPort(m_server_port);
					m_server_port = nullptr;
				}

				di("exit - coms shutdown");
			}

			NTSTATUS send_message_sync(
				PVOID send_buffer, ULONG send_buffer_size,
				PVOID reply_buffer, ULONG reply_buffer_size,
				ULONG relative_timeout_ms)
			{
				LARGE_INTEGER timeout;
				timeout.QuadPart = -((LONGLONG)relative_timeout_ms * 10 * 1000);

				if (!m_filter_handle)
				{
					de("Failed send_message - invalid filter handle ");
					return STATUS_INVALID_PARAMETER;
				}

				if (!m_client_port)
				{
					de("Failed send_message - invalid client port");
					return STATUS_INVALID_PARAMETER;
				}

				if (!send_buffer || !send_buffer_size)
				{
					de("Failed send_message - invalid send buffer");
					return STATUS_INVALID_PARAMETER;
				}

				// timeout is in relative 100 nanosecond ticks
				NTSTATUS status = FltSendMessage(
					m_filter_handle,
					&m_client_port,        // this must be a valid PORT handle initialized via FltCreateCommunicationPort
					send_buffer,
					send_buffer_size,
					reply_buffer,
					&reply_buffer_size,
					&timeout);

				if (STATUS_SUCCESS != status) {
					de("Failed sendign message - %ul", status);
				}

				return status;
			};

			NTSTATUS send_message_async(
				PVOID send_buffer, ULONG send_buffer_size,
				PVOID reply_buffer, ULONG reply_buffer_size,
				PLARGE_INTEGER timeout)
			{
				//todo
				UNREFERENCED_PARAMETER(send_buffer);
				UNREFERENCED_PARAMETER(send_buffer_size);
				UNREFERENCED_PARAMETER(reply_buffer);
				UNREFERENCED_PARAMETER(reply_buffer_size);
				UNREFERENCED_PARAMETER(timeout);

				return STATUS_INVALID_PARAMETER;
			}

			static NTSTATUS callback_on_connect(
				PFLT_PORT client_port,
				PVOID server_port_cookie,
				PVOID connection_context,
				ULONG size_of_context,
				PVOID* connection_cookie
			)
			{
				di("enter callback on connect");

				UNREFERENCED_PARAMETER(size_of_context);
				UNREFERENCED_PARAMETER(connection_context);

				//todo: add some validation to check the other end point
				//trust validation would be great to implement using a signature

				kernel_user_com* connection_obj = (kernel_user_com*)(server_port_cookie);
				if (!connection_obj) {
					de("Failed to obtain server_cookie, an access to the underlying object is required to proceed...");
					return STATUS_FAIL_CHECK;
				}

				//this here is basically done to transfer the "this" pointer to all 
				//calling context so we can operate as required. 

				*connection_cookie = server_port_cookie;
				connection_obj->m_client_port = client_port;

				di("exit callback on connect");
				return STATUS_SUCCESS;
			}

			static VOID callback_on_disconnect(
					_In_opt_ PVOID connection_cookie
				)
			{	
				di("enter callback on disconnect");

				kernel_user_com* connection_obj = (kernel_user_com*)(connection_cookie);
				if (!connection_obj) {
					de("Failed to obtain connection_cookie, an access to the underlying object is required to proceed...");
				}
				else {
					::FltCloseClientPort(
						connection_obj->m_filter_handle, 
						&connection_obj->m_client_port);

					connection_obj->m_client_port = nullptr;
				}

				di("exit callback on disconnect");
			}

			static NTSTATUS callback_on_message(
					_In_ PVOID port_cookie,
					_In_reads_bytes_opt_(input_buffer_size) PVOID input_buffer,
					_In_ ULONG input_buffer_size,
					_Out_writes_bytes_to_opt_(output_buffer_size, *return_output_buffer_length) PVOID output_buffer,
					_In_ ULONG output_buffer_size,
					_Out_ PULONG return_output_buffer_length
				)
			{
				di("enter callback on message");

				UNREFERENCED_PARAMETER(input_buffer_size);
				UNREFERENCED_PARAMETER(output_buffer_size);

				kernel_user_com* connection_obj = (kernel_user_com*)(port_cookie);
				if (!connection_obj) {
					de("Failed to obtain port_cookie, an access to the underlying object is required to proceed...");
					return STATUS_INVALID_PARAMETER;
				}

				if (!input_buffer || !output_buffer) {
					de("Empty buffer no message to process...");
					return STATUS_INVALID_PARAMETER;
				}

				//TODO: update this 
				output_buffer = nullptr;
				return_output_buffer_length = nullptr;


				//process the message

				di("exit callback on message");

				return STATUS_SUCCESS;
			}
		};
	};
