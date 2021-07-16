/**
 * Copyright (c) 2018 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

/* Ref: https://github.com/rxi/uuid4 */
#ifndef UUID_H
#define UUID_H

#define UUID_VERSION "1.0.0"
#define UUID_STR_LEN 37

typedef unsigned char uuid_t[16];

void uuid_generate(uuid_t out);
void uuid_unparse(const uuid_t uu, char *out);

#endif
