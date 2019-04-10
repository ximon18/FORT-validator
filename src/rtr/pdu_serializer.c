#include "pdu_serializer.h"

#include <stdlib.h>
#include <string.h>
#include "primitive_writer.h"

void
init_buffer(struct data_buffer *buffer)
{
	buffer->capacity = BUFFER_SIZE;
	buffer->data = malloc(BUFFER_SIZE);
}

void
free_buffer(struct data_buffer *buffer)
{
	free(buffer->data);
}

static size_t
serialize_pdu_header(struct pdu_header *header, uint16_t union_value, char *buf)
{
	char *ptr;

	ptr = buf;
	ptr = write_int8(ptr, header->protocol_version);
	ptr = write_int8(ptr, header->pdu_type);
	ptr = write_int16(ptr, union_value);
	ptr = write_int32(ptr, header->length);

	return ptr - buf;
}

size_t
serialize_serial_notify_pdu(struct serial_notify_pdu *pdu, char *buf)
{
	size_t head_size;
	char *ptr;

	head_size = serialize_pdu_header(&pdu->header, pdu->header.m.session_id,
	    buf);

	ptr = buf + head_size;
	ptr = write_int32(ptr, pdu->serial_number);

	return ptr - buf;
}

size_t
serialize_cache_response_pdu(struct cache_response_pdu *pdu, char *buf)
{
	/* No payload to serialize */
	return serialize_pdu_header(&pdu->header, pdu->header.m.session_id,
	    buf);
}

size_t
serialize_ipv4_prefix_pdu(struct ipv4_prefix_pdu *pdu, char *buf)
{
	size_t head_size;
	char *ptr;

	head_size = serialize_pdu_header(&pdu->header, pdu->header.m.reserved,
	    buf);

	ptr = buf + head_size;
	ptr = write_int8(ptr, pdu->flags);
	ptr = write_int8(ptr, pdu->prefix_length);
	ptr = write_int8(ptr, pdu->max_length);
	ptr = write_int8(ptr, pdu->zero);
	ptr = write_in_addr(ptr, pdu->ipv4_prefix);
	ptr = write_int32(ptr, pdu->asn);

	return ptr - buf;
}

size_t
serialize_ipv6_prefix_pdu(struct ipv6_prefix_pdu *pdu, char *buf)
{
	size_t head_size;
	char *ptr;

	head_size = serialize_pdu_header(&pdu->header, pdu->header.m.reserved,
	    buf);

	ptr = buf + head_size;
	ptr = write_int8(ptr, pdu->flags);
	ptr = write_int8(ptr, pdu->prefix_length);
	ptr = write_int8(ptr, pdu->max_length);
	ptr = write_int8(ptr, pdu->zero);
	ptr = write_in6_addr(ptr, pdu->ipv6_prefix);
	ptr = write_int32(ptr, pdu->asn);

	return ptr - buf;
}

size_t
serialize_end_of_data_pdu(struct end_of_data_pdu *pdu, char *buf)
{
	size_t head_size;
	char *ptr;

	head_size = serialize_pdu_header(&pdu->header, pdu->header.m.session_id,
	    buf);

	ptr = buf + head_size;
	ptr = write_int32(ptr, pdu->serial_number);
	if (pdu->header.protocol_version == RTR_V1) {
		ptr = write_int32(ptr, pdu->refresh_interval);
		ptr = write_int32(ptr, pdu->retry_interval);
		ptr = write_int32(ptr, pdu->expire_interval);
	}

	return ptr - buf;
}

size_t
serialize_cache_reset_pdu(struct cache_reset_pdu *pdu, char *buf)
{
	/* No payload to serialize */
	return serialize_pdu_header(&pdu->header, pdu->header.m.reserved, buf);
}

size_t
serialize_error_report_pdu(struct error_report_pdu *pdu, char *buf)
{
	struct pdu_header *err_pdu_header;
	size_t head_size;
	char *ptr, *tmp_ptr;
	int i;

	head_size = serialize_pdu_header(&pdu->header, pdu->header.m.error_code,
	    buf);

	ptr = buf + head_size;

	ptr = write_int32(ptr, pdu->error_pdu_length);
	if (pdu->error_pdu_length > 0) {
		/* Set only the header of err PDU */
		err_pdu_header = (struct pdu_header *)pdu->erroneous_pdu;
		head_size = serialize_pdu_header(err_pdu_header,
		    err_pdu_header->m.reserved, ptr);
		ptr = ptr + head_size;
	}

	ptr = write_int32(ptr, pdu->error_message_length);
	tmp_ptr = pdu->error_message;
	for (i = 0; i < pdu->error_message_length; i++)
		ptr = write_int8(ptr, tmp_ptr[i]);

	return ptr - buf;
}