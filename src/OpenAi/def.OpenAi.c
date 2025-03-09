//silver_chain_scope_start
//DONT MODIFY THIS COMMENT
//this import is computationally generated
//mannaged by SilverChain: https://github.com/OUIsolutions/SilverChain
#include "../imports/imports.dec.h"
//silver_chain_scope_end



OpenAiInterface * newOpenAiInterface(const char *url, const char *apiKey,const char *model){
    OpenAiInterface *self = (OpenAiInterface*)BearsslHttps_allocate(sizeof(OpenAiInterface));
    self->request = newBearHttpsRequest(url);
    self->max_retrys = OPEN_AI_SDK_MAX_RETRY_TIMES;
    BearHttpsRequest_set_method(self->request, "POST");
    BearHttpsRequest_add_header_fmt(self->request, "Authorization", "Bearer %s",apiKey);
    //set cache to 0
    self->body_object = BearHttpsRequest_create_cJSONPayloadObject(self->request);
    self->messages = cJSON_CreateArray();
    OpenAiInterface_set_model(self, model);
    cJSON_AddItemToObject(self->body_object, "messages", self->messages);
    
    return self;
}

void OpenAiInterface_set_temperature(OpenAiInterface *self, float temperature){
    cJSON_DeleteItemFromObjectCaseSensitive(self->body_object, "temperature");
    cJSON_AddNumberToObject(self->body_object, "temperature", temperature);
}

void OpenAiInterface_set_max_tokens(OpenAiInterface *self, float temperature){
    cJSON_DeleteItemFromObjectCaseSensitive(self->body_object, "max_tokens");
    cJSON_AddNumberToObject(self->body_object, "max_tokens", temperature);
}
void OpenAiInterface_set_model(OpenAiInterface *self, const char *model){
    cJSON_DeleteItemFromObjectCaseSensitive(self->body_object, "model");
    cJSON_AddStringToObject(self->body_object, "model", model);
}

void OpenAiInterface_add_raw_prompt(OpenAiInterface *self,const char *role, const char *prompt){
    cJSON *prompt_object = cJSON_CreateObject();
    cJSON_AddStringToObject(prompt_object, "role", role);
    cJSON_AddStringToObject(prompt_object, "content", prompt);
    cJSON_AddItemToArray(self->messages, prompt_object);
}

void OpenAiInterface_add_system_prompt(OpenAiInterface *self, const char *prompt){
    OpenAiInterface_add_raw_prompt(self, "system", prompt);
}

void OpenAiInterface_add_user_prompt(OpenAiInterface *self, const char *prompt){
    OpenAiInterface_add_raw_prompt(self, "user", prompt);
}

OpenAiAnswer * OpenAiInterface_make_question(OpenAiInterface *self){
  
    for(int i = 0; i  < self->max_retrys;i++){
        BearHttpsResponse *response =BearHttpsRequest_fetch(self->request);
        const char *body_str = BearHttpsResponse_read_body_str(response);
        if(BearHttpsResponse_error(response)){
            char *error = BearHttpsResponse_get_error_msg(response);
            return private_newOpenAiAnswer_error(response, NULL, error);
        }
        if(BearHttpsResponse_get_body_size(response) == 0){
            BearHttpsResponse_free(response);
            continue;
        }
        cJSON *body = cJSON_Parse(body_str);
        if(cJSON_GetObjectItemCaseSensitive(body, "error") != NULL){
            char *error = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(body, "error"));
            return private_newOpenAiAnswer_error(response,body, error);
        }
        return private_newOpenAiAnswer_ok(response, body);
        
    }
    return private_newOpenAiAnswer_error(NULL,NULL, "Max retry times reached");

}