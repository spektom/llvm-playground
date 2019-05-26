#ifndef MACROS_H_
#define MACROS_H_

// Macros to disable copying and moving

#define DISALLOW_COPY(cname)                                                   \
  cname(const cname &) = delete;                                               \
  cname &operator=(const cname &) = delete;

#define DISALLOW_MOVE(cname)                                                   \
  cname(cname &&) = delete;                                                    \
  cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname)                                          \
  DISALLOW_COPY(cname);                                                        \
  DISALLOW_MOVE(cname);

#endif /* MACROS_H_ */
