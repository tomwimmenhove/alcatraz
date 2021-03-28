#ifndef DUMMY_H
#define DUMMY_H

#ifdef GUEST_CUSTOM_WRITE
#define GUEST_CUSTOM_WRITE_CODE
#else
#define GUEST_CUSTOM_WRITE_CODE int64_t write(int fd, const void *buf, size_t count) { return 0; }
#endif

#ifdef GUEST_CUSTOM_READ
#define GUEST_CUSTOM_READ_CODE
#else
#define GUEST_CUSTOM_READ_CODE int64_t read(int fd, const void *buf, size_t count) { return 0; }
#endif

#ifdef GUEST_CUSTOM_CLOSE
#define GUEST_CUSTOM_CLOSE_CODE
#else
#define GUEST_CUSTOM_CLOSE_CODE int close(int) { return 0; }
#endif

#define DEFINE_GUEST_DUMMY_CALLS	\
extern "C" {						\
	GUEST_CUSTOM_WRITE_CODE			\
	GUEST_CUSTOM_READ_CODE			\
	GUEST_CUSTOM_CLOSE_CODE			\
}

#endif /* DUMMY_H */
