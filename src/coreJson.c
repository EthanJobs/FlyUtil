#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <coreJson.h>
#include <coreLink.h>
#include <coreIterator.h>

// 比较两个jv的大小 strcmp两个jv的n_name
int compareJVToJV(void *a, void *b);

// 比较字符串和jv的大小 strcmp字符串和jv的n_name
int compareStringToJV(void *a, void *b);

int judgeStringByFd(int fd, int *state, char *tmp, char *str);

// 读取一部分字符串并返回一个dataType为JSONINT或JSONDOUBLE的jsonData
jsonData jsonData_returnNumByString(char *jsonStr, int *index, int *fd);
int jsonData_returnNumByFd(jsonData *jd, int fd, int *state, char *tmp, int *dt);

// 读取一部分字符串并返回一个用于作为一个dataType为JSONSTR的jsonData的n_data值
char *jsonData_returnStrByString(char *jsonStr, int *index);
char *jsonData_returnStrByFd(int fd, int *state, char *tmp);

// 读取一部分字符串并返回一个相应的jsonValue
jsonValue *jsonValue_creatByString(char *jsonStr, int *index, int cond);
jsonValue *jsonValue_creatByFd(int fd, int *state, char *tmp, int hasName);

// 读取一部分字符串并返回一个相应的Link
Link *Json_Nums_initByString(char *jsonStr, int *index);
Link *Json_Nums_initByFd(int fd, int *state, char *tmp);

// 读取一部分字符串并返回一个相应的Json
Json *Json_Obj_initByString(char *jsonStr, int *index);
Json *Json_Obj_initByFd(int fd, int *state, char *tmp);

// 无实际意义
void Json_print_in(void *j, int cond);

// 从j中获取相应的jsonValue
jsonValue *Json_getObjValue(Json *j, char *scStr, int *index);

// 从l中获取相应的jsonValue
jsonValue *Json_getNumsValue(Link *l, char *scStr, int *index);

// TODO: FunCompare
int compareJVToJV(void *a, void *b) {
    return strcmp(((jsonValue *)a)->n_name, ((jsonValue *)b)->n_name);
}

int compareStringToJV(void *a, void *b) {
    return strcmp((char *)a, ((jsonValue *)b)->n_name);
}

// TODO: JsonValue
jsonValue *jsonValue_init(char *name, jsonData jd, int dt) {
    if (dt > 6 || dt < 0) return NULL;

    jsonValue *jv = (jsonValue *)malloc(sizeof(jsonValue));

    jv->n_name = name;
    jv->n_dataType = dt;
    jv->n_data = jd;

    return jv;
}

void jsonValue_free(void *a) {
    jsonValue *jv = (jsonValue *)a;
    if (jv == NULL) return;

    if (jv->n_name != NULL) free(jv->n_name);

    if (jv->n_dataType == JSONSTR) free((char *)jv->n_data.p_data);
    else if (jv->n_dataType == JSONOBJ) Json_free((Json *)jv->n_data.p_data);
    else if (jv->n_dataType == JSONNUMS) Link_free((Link *)jv->n_data.p_data);

    free(jv);

    return;
}

// TODO: Json
Json *Json_init() {
    return Tree_init(compareJVToJV, jsonValue_free);
}

int Json_insertValue(Json *j, jsonValue *jv) {
    return Tree_insertValue(j, (void *)jv);
}

void Json_free(Json *j) {
    Tree_free(j);
}

// TODO: GetValue
jsonValue *Json_getObjValue(Json *j, char *scStr, int *index) {
    for (; scStr[(*index)] == ' ' || scStr[(*index)] == '\n' || scStr[(*index)] == '\r'; (*index)++);

    if (scStr[*index] == '{') {
        (*index)++;
        int tmp;
        for (tmp = *index; scStr[tmp] != '[' && scStr[tmp] != '{' && scStr[tmp] != '}' && scStr[tmp] != ']' && scStr[tmp] != ' ' && scStr[tmp] != '\0'; tmp++);
        char *searchName = jsonData_returnStrByString(scStr, index);
        if (searchName == NULL) return NULL;
        jsonValue *jv = (jsonValue *)Tree_getValue(j, (void *)searchName, compareStringToJV);
        if (searchName != NULL) {
            free(searchName);
            searchName = NULL;
        }
        if (jv == NULL) return NULL;
        *index = tmp;
        for (; scStr[(*index)] == ' ' || scStr[(*index)] == '\n' || scStr[(*index)] == '\r'; (*index)++);
        if (jv->n_dataType == JSONOBJ && scStr[*index] == '{')
            return Json_getObjValue((Json *)jv->n_data.p_data, scStr, index);
        if (jv->n_dataType == JSONNUMS && scStr[*index] == '[')
            return Json_getNumsValue((Link *)jv->n_data.p_data, scStr, index);
        else
            return jv;
    } else {
        return NULL;
    }
}

jsonValue *Json_getNumsValue(Link *l, char *scStr, int *index) {
    for (; scStr[(*index)] == ' ' || scStr[(*index)] == '\n' || scStr[(*index)] == '\r'; (*index)++);

    if (scStr[*index] == '[') {
        (*index)++;
        int num = 0;
        for (; scStr[*index] >= '0' && scStr[*index] <= '9'; (*index)++) num = num * 10 + (scStr[*index] - '0');
        jsonValue *jv = (jsonValue *)Link_getValue(l, num);
        if (jv == NULL) return NULL;
        for (; scStr[(*index)] == ' ' || scStr[(*index)] == '\n' || scStr[(*index)] == '\r'; (*index)++);
        if (jv->n_dataType == JSONOBJ && scStr[*index] == '{')
            return Json_getObjValue((Json *)jv->n_data.p_data, scStr, index);
        if (jv->n_dataType == JSONNUMS && scStr[*index] == '[')
            return Json_getNumsValue((Link *)jv->n_data.p_data, scStr, index);
        else
            return jv;
    } else {
        return NULL;
    }
}

jsonValue *Json_getValueByQueryJson(Json *j, char *queryJson) {
    if (!j || !queryJson) return NULL;

    int index = 0;
    return Json_getObjValue(j, queryJson, &index);
}

jsonValue *Json_getValueInObj(Json *j, char *name) {
    if (!j || !name) return NULL;

    return (jsonValue *)Tree_getValue(j, (void *)name, compareStringToJV);
}

jsonValue *Json_getValueInNums(Link *l, int index) {
    if (!l) return NULL;

    return (jsonValue *)Link_getValue(l, index);
}

// TODO: String
jsonData jsonData_returnNumByString(char *jsonStr, int *index, int *fd) {
    int i;
    jsonData ret;
    ret.i_data = 0;
    for (; jsonStr[*index] >= '0' && jsonStr[*index] <= '9'; (*index)++)
        ret.i_data = ret.i_data * 10 + (jsonStr[*index] - '0');
    *fd = JSONINT;
    if (jsonStr[*index] == '.') {
        *fd = JSONDOUBLE;
        (*index)++;
        ret.d_data = (double)ret.i_data;
        double l = 0, tmp = 10;
        for (; jsonStr[*index] >= '0' && jsonStr[*index] <= '9'; (*index)++, tmp *= 10)
            l = l + (jsonStr[*index] - '0') / tmp;
        ret.d_data += l;
    }
    return ret;
}

char *jsonData_returnStrByString(char *jsonStr, int *index) {
    if (jsonStr[(*index)++] != '\"') return NULL;
    int tmp = (*index);
    while (jsonStr[(*index)] != '\"')
        if (jsonStr[(*index)++] == '\0') return NULL;
    char *retStr = (char *)malloc(sizeof(char) * ((*index) - tmp));
    int i;
    for (i = 0; jsonStr[tmp] != '\"'; i++)
        retStr[i] = jsonStr[tmp++];
    (*index)++;
    retStr[i] = '\0';
    return retStr;
}

jsonValue *jsonValue_creatByString(char *jsonStr, int *index, int hasName) {
    for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++);

    char *name = NULL;
    if (hasName) {
        name = jsonData_returnStrByString(jsonStr, index);
        if (!name) return NULL;

        for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r' || jsonStr[(*index)] == ':'; (*index)++);
    }

    int dt;
    jsonData jd;
    if (jsonStr[(*index)] >= '0' && jsonStr[(*index)] <= '9') {
        jd = jsonData_returnNumByString(jsonStr, index, &dt);
    } else if (jsonStr[(*index)] == '\"') {
        dt = JSONSTR;
        jd.p_data = (void *)jsonData_returnStrByString(jsonStr, index);
    } else if (jsonStr[(*index)] == 't' || jsonStr[(*index)] == 'f') {
        // bool值只支持小写 暂时
        dt = JSONBOOL;
        if (strncmp(jsonStr + (*index), "true", 4) == 0) {
            jd.p_data = (void *)"true";
            (*index) += 4;
        } else if (strncmp(jsonStr + (*index), "false", 5) == 0) {
            jd.p_data = (void *)"false";
            (*index) += 5;
        } else {
            free(name);
            name = NULL;
            return NULL;
        }
    } else if (jsonStr[(*index)] == 'n') {
        dt = JSONNULL;
        if (strncmp(jsonStr + (*index), "null", 4) == 0) {
            jd.p_data = (void *)"null";
            (*index) += 4;
        } else {
            free(name);
            name = NULL;
            return NULL;
        }
    } else if (jsonStr[(*index)] == '[') {
        dt = JSONNUMS;
        jd.p_data = (void *)Json_Nums_initByString(jsonStr, index);
    } else if (jsonStr[(*index)] == '{') {
        dt = JSONOBJ;
        jd.p_data = (void *)Json_Obj_initByString(jsonStr, index);
    } else {
        free(name);
        name = NULL;
        return NULL;
    }

    return jsonValue_init(name, jd, dt);
}

Link *Json_Nums_initByString(char *jsonStr, int *index) {
    for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++); //剔除空格
    if (jsonStr[(*index)] != '[')
        return NULL;
    Link *l = Link_init(jsonValue_free);
    (*index)++;
    while (1) {
        for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++);
        jsonValue *myjv = jsonValue_creatByString(jsonStr, index, False);
        Link_tailInsertValue(l, (void *)myjv);
        for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++);
        if (jsonStr[(*index)] == ',') {
            (*index)++;
            continue;
        } else if (jsonStr[(*index)] == ']') {
            (*index)++;
            return l;
        } else {
            Link_free(l);
            return NULL;
        }
    }
}

Json *Json_Obj_initByString(char *jsonStr, int *index) {
    for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++); //剔除空格
    if (jsonStr[(*index)] != '{')
        return NULL;

    Json *j = Json_init();

    while (1) {
        for ((*index)++; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++);
        jsonValue *jv = jsonValue_creatByString(jsonStr, index, True);
        if (jv == NULL) break;
        Json_insertValue(j, jv);
        for (; jsonStr[(*index)] == ' ' || jsonStr[(*index)] == '\n' || jsonStr[(*index)] == '\r'; (*index)++);
        if (jsonStr[(*index)] == ',') continue;
        if (jsonStr[(*index)] == '}') {
            (*index)++;
            return j;
        }
        Json_free(j);
        return NULL;
    }

}

Json *Json_initByString(char *jsonStr) {
    if (!jsonStr)
        return NULL;
    int index = 0;
    return Json_Obj_initByString(jsonStr, &index);
}

// TODO: File Descriptor
int judgeStringByFd(int fd, int *state, char *tmp, char *str) {
    int i;
    for (i = 0; state > 0 && *tmp == str[i]; i++, *state = read(fd, tmp, 1));
    if (i == strlen(str)) return 0;
    return 1;
}

int jsonData_returnNumByFd(jsonData *jd, int fd, int *state, char *tmp, int *dt) {
    jd->i_data = 0;

    for (; *state > 0 && *tmp >= '0' && *tmp <= '9'; *state = read(fd, tmp, 1))
        jd->i_data = jd->i_data * 10 + (*tmp - '0');
    if (state <= 0) return 1;

    *dt = JSONINT;
    if (*tmp == '.') {
        *dt = JSONDOUBLE;
        jd->d_data = (double)jd->i_data;
        double l = 0, dTmp = 10;
        for (*state = read(fd, tmp, 1); *tmp >= '0' && *tmp <= '9'; *state = read(fd, tmp, 1), dTmp *= 10)
            l = l + (*tmp - '0') / dTmp;
        jd->d_data += l;
    }
    return 0;
}

char *jsonData_returnStrByFd(int fd, int *state, char *tmp) {
    if (*tmp != '\"') return NULL;

    char buf[JSON_STR_MAX_SIZE];
    int i = 0;

    for (*state = read(fd, tmp, 1); *state > 0 && *tmp != '\"' && i < JSON_STR_MAX_SIZE; i++, *state = read(fd, tmp, 1))
        buf[i] = *tmp;
    if (*state <= 0) return NULL;

    buf[i++] = '\0';
    char *retStr = (char *)malloc(sizeof(char) * (++i));
    strncpy(retStr, buf, i);

    return retStr;
}

jsonValue *jsonValue_creatByFd(int fd, int *state, char *tmp, int hasName) {
    for (; *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
    if (*state <= 0) return NULL;

    char *name = NULL;
    if (hasName) {
        name = jsonData_returnStrByFd(fd, state, tmp);
        if (!name) return NULL;

        for (*state = read(fd, tmp, 1); *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
        if (*state <= 0 || *tmp != ':') return NULL;

        for (*state = read(fd, tmp, 1); *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
        if (*state <= 0) return NULL;
    }

    int dt, ret;
    jsonData jd;
    while (1) {
        if (*tmp >= '0' && *tmp <= '9') {
            ret = jsonData_returnNumByFd(&jd, fd, state, tmp, &dt);
            if (ret) break;
        } else if (*tmp == '\"') {
            dt = JSONSTR;
            jd.p_data = (void *)jsonData_returnStrByFd(fd, state, tmp);
            if ((char *)jd.p_data == NULL) break;
        } else if (*tmp == 't') {
            if (judgeStringByFd(fd, state, tmp, "true")) break;
            dt = JSONBOOL;
            jd.p_data = (void *)"true";
        } else if (*tmp == 'f') {
            if (judgeStringByFd(fd, state, tmp, "false")) break;
            dt = JSONBOOL;
            jd.p_data = (void *)"false";
        } else if (*tmp == 'n') {
            if (judgeStringByFd(fd, state, tmp, "null")) break;
            dt = JSONBOOL;
            jd.p_data = (void *)"FALSE";
        } else if (*tmp == '[') {
            dt = JSONNUMS;
            jd.p_data = (void *)Json_Nums_initByFd(fd, state, tmp);
            if ((Link *)jd.p_data == NULL) break;
        } else if (*tmp == '{') {
            dt = JSONOBJ;
            jd.p_data = (void *)Json_Obj_initByFd(fd, state, tmp);
            if ((Json *)jd.p_data == NULL) break;
        } else {
            break;
        }

        return jsonValue_init(name, jd, dt);
    }

    free(name);
    return NULL;
}

Link *Json_Nums_initByFd(int fd, int *state, char *tmp) {
    for (; *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
    if (*state <= 0 || *tmp != '[') return NULL;

    Link *l = Link_init(jsonValue_free);
    while (1) {
        for (*state = read(fd, tmp, 1); *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
        if (*state <= 0) break;

        jsonValue *jv = jsonValue_creatByFd(fd, state, tmp, False);
        if (jv == NULL) break;
        Link_tailInsertValue(l, (void *)jv);

        for (*state = read(fd, tmp, 1); *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
        if (*state <= 0) break;

        if (*tmp == ',') continue;
        if (*tmp == ']') return l;
    }

    Link_free(l);
    return NULL;
}

Json *Json_Obj_initByFd(int fd, int *state, char *tmp) {
    for (; *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
    if (*state <= 0 || *tmp != '{') return NULL;

    Json *j = Json_init();

    while (1) {
        for (*state = read(fd, tmp, 1); *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
        if (*state <= 0) break;

        jsonValue *jv = jsonValue_creatByFd(fd, state, tmp, True);
        if (jv == NULL) break;
        Json_insertValue(j, jv);

        for (*state = read(fd, tmp, 1); *state > 0 && (*tmp == ' ' || *tmp == '\n' || *tmp == '\r'); *state = read(fd, tmp, 1));
        if (*state <= 0) break;

        if (*tmp == ',') continue;
        if (*tmp == '}') return j;
        break;
    }

    Json_free(j);
    return NULL;
}

Json *Json_initByFd(int fd) {
    if (fd < 0) return NULL;
    int state;
    char tmp;
    state = read(fd, &tmp, 1);
    return Json_Obj_initByFd(fd, &state, &tmp);
}

// TODO: print
void jsonValue_print(jsonValue *jv) {
    if (!jv) return;
    if (jv->n_name) printf("\"%s\" : ", jv->n_name);
    if (jv->n_dataType == JSONINT) {
        printf("%d", jv->n_data.i_data);
    } else if (jv->n_dataType == JSONDOUBLE) {
        printf("%lf", jv->n_data.d_data);
    } else if (jv->n_dataType == JSONSTR) {
        printf("\"%s\"", (char *)jv->n_data.p_data);
    } else if (jv->n_dataType == JSONBOOL || jv->n_dataType == JSONNULL) {
        printf("%s", (char *)jv->n_data.p_data);
    } else if (jv->n_dataType == JSONNUMS) {
        Json_print_in(jv->n_data.p_data, 1);
    } else if (jv->n_dataType == JSONOBJ) {
        Json_print_in(jv->n_data.p_data, 0);
    }
}

void printJsonValue(void *a) {
    jsonValue *jv = (jsonValue *)a;
    if (!jv) return;
    if (jv->n_name) printf("\"%s\" : ", jv->n_name);
    if (jv->n_dataType == JSONINT) {
        printf("%d", jv->n_data.i_data);
    } else if (jv->n_dataType == JSONDOUBLE) {
        printf("%lf", jv->n_data.d_data);
    } else if (jv->n_dataType == JSONSTR) {
        printf("\"%s\"", (char *)jv->n_data.p_data);
    } else if (jv->n_dataType == JSONBOOL || jv->n_dataType == JSONNULL) {
        printf("%s", (char *)jv->n_data.p_data);
    } else if (jv->n_dataType == JSONNUMS) {
        Json_print_in(jv->n_data.p_data, 1);
    } else if (jv->n_dataType == JSONOBJ) {
        Json_print_in(jv->n_data.p_data, 0);
    }
    printf(", ");
}

void Json_print_in(void *j, int cond) {
    if (!j) {
        printf("Format error!\n");
        return;
    }

    if (cond) {
        printf("[");
        Iterator *i = Link_getIterator((Link *)j);
        while (i->fun_hasNext(i)) {
            jsonValue_print((jsonValue *)i->fun_next(i));
            if (i->fun_hasNext(i)) printf(", ");
        }
        Iterator_free(i);
        printf("]");
    } else {
        printf("{");
        Iterator *i = Tree_getInorderIterator((Json *)j);
        while (i->fun_hasNext(i)) {
            jsonValue_print((jsonValue *)i->fun_next(i));
            if (i->fun_hasNext(i)) printf(", ");
        }
        Iterator_free(i);
        printf("}");
    }

    return;
}

void Json_displayValue(Json *j) {
    Json_print_in((void *)j, 0);
    putchar(10);

    return;
}
