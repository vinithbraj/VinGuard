#pragma once

extern "C"
{
#include <fltKernel.h>
#include "VinGuardDebugMacros.h"
#pragma comment(lib, "fltmgr.lib")
}

extern "C"
{
	namespace VG
	{
		const wchar_t port_name[] = L"\\c7fe3b44-4759-4a07-a37a-ef89a9316096";
		const int max_connections = 1;
		class kernel_user_com {

		public:

			PFLT_PORT m_server_port;
			PFLT_PORT m_client_port;
			PFLT_FILTER m_filter_handle;

			kernel_user_com(PFLT_FILTER filter_handle) :
				m_filter_handle(filter_handle),
				m_server_port(nullptr),
				m_client_port(nullptr)
			{

			}

			bool initialize() {

				OBJECT_ATTRIBUTES port_attributes = { NULL };
				const ULONG flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
				UNICODE_STRING l_port_name = RTL_CONSTANT_STRING(port_name);

				InitializeObjectAttributes(
					&port_attributes,
					&l_port_name,
					flags, 0, 0);


				auto result = FltCreateCommunicationPort(
					m_filter_handle,
					&m_server_port,
					&port_attributes,
					(PVOID)this,
					kernel_user_com::callback_on_connect,
					kernel_user_com::callback_on_disconnect,
					kernel_user_com::callback_on_message,
					max_connections);
			}

			bool shutdown() {

			}

			static NTSTATUS callback_on_connect(
				PFLT_PORT client_port,
				PVOID server_port_cookie,
				PVOID connection_context,
				ULONG size_of_context,
				PVOID* connection_cookie
			)
			{
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
				return STATUS_SUCCESS;
			}

			static VOID callback_on_disconnect(
					_In_opt_ PVOID connection_cookie
				)
			{	
				di("Closing port");

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
				kernel_user_com* connection_obj = (kernel_user_com*)(port_cookie);
				if (!connection_obj) {
					de("Failed to obtain port_cookie, an access to the underlying object is required to proceed...");
					return STATUS_INVALID_PARAMETER;
				}

				if (!input_buffer || !output_buffer) {
					de("Empty buffer no message to process...");
					return STATUS_INVALID_PARAMETER;
				}

				//process the message
				return STATUS_SUCCESS;
			}
		};
	}
}
