#include "../helpers/myfstree.h"

time_t t;

myfstree * root = NULL;

char * extract_path(char ** copy_path){
    char * return_value = (char *)calloc(sizeof(char), 1);
    int return_len = 0;
    char temporary;
    char * tempstr;
    temporary = **(copy_path);
    while(temporary != '\0'){
        if(temporary == '/'){
            if(strlen(*copy_path) > 1){
                (*copy_path)++;
            }
            break;
        }
        tempstr = (char *)calloc(sizeof(char) , (return_len + 2));
        strcpy(tempstr, return_value);
        return_len += 1;
        tempstr[return_len - 1] = temporary;
        return_value = (char *)realloc(return_value, sizeof(char) * (return_len + 2));
        strcpy(return_value, tempstr);
        (*copy_path)++;
        temporary = **(copy_path);
        free(tempstr);
    }
    return_value = (char *)realloc(return_value, sizeof(char) * (return_len + 1));
    return_value[return_len] = '\0';
    return return_value;
}

char * reverse(char * str, int mode){
    int i;
    int len = strlen(str);
    char * return_value = (char *)calloc(sizeof(char), (len + 1));
    for(i = 0; i <= len/2; i++){
        return_value[i] = str[len - 1 -i];
        return_value[len - i - 1] = str[i];
    }
    if(return_value[0] == '/' && mode == 1) {
        return_value++;
    }
    return return_value;
}

char * extract_dir(char ** copy_path){
    char * return_value = (char *)calloc(sizeof(char), 1);
    int return_len = 0;
    char temporary;
    char * tempstr;
    *copy_path = reverse(*copy_path, 1);
    temporary = **(copy_path);
    while(temporary != '/'){
        tempstr = (char *)calloc(sizeof(char), (return_len + 2));
        strcpy(tempstr, return_value);
        return_len += 1;
        tempstr[return_len - 1] = temporary;
        return_value = (char *)realloc(return_value, sizeof(char) * (return_len + 1));
        strcpy(return_value, tempstr);
        (*copy_path)++;
        temporary = **(copy_path);
        free(tempstr);
    }
    if(strlen(*copy_path) > 1){
        (*copy_path)++;
    }
    return_value = (char *)realloc(return_value, sizeof(char) * (return_len + 1));
    return_value[return_len] = '\0';
    return_value = reverse(return_value, 0);
    *(copy_path) = reverse(*(copy_path), 0);
    return return_value;
}

myfstree * search_node(char * path){
    myfstree * temporary = root;
    myfstree * return_value = NULL;
    char * curr_node = NULL;
    int flag = 0, i = 0;
    if(path[0] == '/'){
        path++;
    }
    while(temporary != NULL){
        curr_node = extract_path(&path);
        if(strlen(curr_node) == 0){
            break;
        }
        for(i = 0; i < temporary->num_children; i++){
            if(strcmp(temporary->children[i]->name, curr_node) == 0){
                return_value = temporary->children[i];
                temporary = temporary->children[i];
                flag = 1;
            }
        }
        if(!flag){
            return NULL;
        }
        else{
            flag = 0;
        }
    }
    if(return_value!=NULL){
    	return return_value;
	}
	return NULL;
}

myfstree * init_node(const char * path, char * name, myfstree * parent,int type){
    myfstree * new = (myfstree *)malloc(sizeof(myfstree));
    new->path = (char *)calloc(sizeof(char), strlen(path) + 1);
    new->name = (char *)calloc(sizeof(char), strlen(name) + 1);
    strcpy(new->path, (char *)path);
    strcpy(new->name, (char *)name);
    if(type == 1){
    	new->type = "directory";  
        new->permissions = S_IFDIR | 0777;
    }     
    if(type == 0){
	    new->type = "file"; 
    	new->permissions = S_IFREG | 0777; 
    }    
    new->group_id = getgid();
    new->user_id = getuid();
    new->c_time = time(&t);
    new->a_time = time(&t);
    new->m_time = time(&t);
    new->b_time = time(&t);
	new->inode_number = 0;
    new->num_children = 0;
    new->parent = parent;
    new->children = NULL;
    new->fchildren = NULL;
    new->num_files = 0;
    new->size = 0;
    return new;
}

void insert_node(const char * path){
    if(root == NULL){
        root = init_node("/", "root", NULL,1);
        return;
    }
    else{
        char * copy_path = (char *)path;
        char * dir = extract_dir(&copy_path);
        myfstree * d_node = NULL;
        if(strlen(copy_path) == 1){   
            root->num_children++;
            if(root->children == NULL){
                root->children = (myfstree **)malloc(sizeof(myfstree *));
                root->children[0] = init_node(path, dir, root,1);
            }
            else{
                root->children = (myfstree **)realloc(root->children, sizeof(myfstree *) * root->num_children);
                root->children[root->num_children - 1] = init_node(path, dir, root,1);
            }
        }
        else{
            d_node = search_node(copy_path);
            if(d_node != NULL){
				if(d_node->parent!=NULL){
					d_node->c_time=time(NULL);
					d_node->m_time=time(NULL);
				}
                d_node->num_children++;
                d_node->children = (myfstree **)realloc(d_node->children, sizeof(myfstree *) * d_node->num_children);
                d_node->children[d_node->num_children - 1] = init_node(path, dir, d_node,1);
            }
        }
        return;
    }
    return;
}

void load_node(char * path, char * type, gid_t groupid, uid_t userid, time_t lc_time, time_t lm_time, time_t la_time, time_t lb_time, unsigned long int inode, off_t size, mode_t lpermissions){
    if(root == NULL){
        root = init_node("/", "root", NULL, 1);
		root->group_id = groupid;
		root->user_id = userid;
		root->c_time = lc_time;
		root->a_time = la_time;
		root->m_time = lm_time;
		root->b_time = lb_time;
		root->inode_number = inode;
		root->size = size;
    }
    else{
        char * copy_path = (char *)path;
        char * dir = extract_dir(&copy_path);
		char * tdir = (char *)calloc(sizeof(char), strlen(dir));
    	strcpy(tdir,dir);
        myfstree * d_node = NULL;
        if(strlen(copy_path) == 1){     
            root->num_children++;
            if(root->children == NULL){
                root->children = (myfstree **)malloc(sizeof(myfstree *));
				if(strcmp(type,"directory")==0){
                	root->children[0] = init_node(path, dir, root,1);
					root->children[0]->permissions=lpermissions;
					root->children[0]->type="directory";
				}
				else{	root->num_files++;
					root->children[0] = init_node(path, dir, root,2);
					root->children[0]->permissions=lpermissions;
					root->fchildren = (myfsfile **)malloc(sizeof(myfsfile *));
					root->fchildren[0] = init_file(path,tdir);
					root->children[0]->type="file";
				}
				root->children[0]->group_id = groupid;
				root->children[0]->user_id = userid;
				root->children[0]->c_time = lc_time;
				root->children[0]->a_time = la_time;
				root->children[0]->m_time = lm_time;
				root->children[0]->b_time = lb_time;
				root->children[0]->inode_number = inode;
				root->children[0]->size = size;
            }
            else{
                root->children = (myfstree **)realloc(root->children, sizeof(myfstree *) * root->num_children);
				if(strcmp(type,"directory")==0){
                	root->children[root->num_children - 1] = init_node(path, dir, root,1);
					root->children[root->num_children - 1]->permissions = lpermissions; 
					root->children[root->num_children - 1]->type = "directory";
				}
				else{
					root->num_files++;
					root->children[root->num_children - 1] = init_node(path, dir, root,2);
					root->children[root->num_children - 1]->permissions= lpermissions; 
					root->fchildren = (myfsfile **)realloc(root->fchildren, sizeof(myfsfile *) * root->num_files);
					root->fchildren[root->num_files - 1] = init_file(path,tdir);
					root->children[root->num_children - 1]->type = "file";
				}
				root->children[root->num_children - 1]->group_id = groupid;
				root->children[root->num_children - 1]->user_id = userid;
				root->children[root->num_children - 1]->c_time = lc_time;
				root->children[root->num_children - 1]->a_time = la_time;
				root->children[root->num_children - 1]->m_time = lm_time;
				root->children[root->num_children - 1]->b_time = lb_time;
				root->children[root->num_children - 1]->inode_number = inode;
				root->children[root->num_children - 1]->size = size;
            }
        }
        else{
            d_node = search_node(copy_path);
            if(d_node != NULL){
				if(d_node->parent!=NULL){
					d_node->c_time=time(NULL);
					d_node->m_time=time(NULL);
				}
                d_node->num_children++;
                d_node->children = (myfstree **)realloc(d_node->children, sizeof(myfstree *) * d_node->num_children);
				if(strcmp(type,"directory")==0){
                	d_node->children[d_node->num_children - 1] = init_node(path, dir, d_node,1);
					d_node->children[d_node->num_children - 1] ->permissions = lpermissions; //S_IFDIR | 0755
					d_node->children[d_node->num_children - 1]->type = "directory";
				}
				else{	d_node->num_files++;
					d_node->children[d_node->num_children - 1] = init_node(path, dir, d_node,2);
					d_node->children[d_node->num_children - 1] ->permissions = lpermissions; //S_IFREG | 0644
					d_node->fchildren = (myfsfile **)realloc(d_node->fchildren, sizeof(myfsfile *) * d_node->num_files);
					d_node->fchildren[d_node->num_files - 1] = init_file(path,dir);
					d_node->children[d_node->num_children - 1]->type = "file";
				}
            }
			d_node->children[d_node->num_children - 1]->group_id = groupid;
			d_node->children[d_node->num_children - 1]->user_id = userid;
			d_node->children[d_node->num_children - 1]->c_time = lc_time;
			d_node->children[d_node->num_children - 1]->a_time = la_time;
			d_node->children[d_node->num_children - 1]->m_time = lm_time;
			d_node->children[d_node->num_children - 1]->b_time = lb_time;
			d_node->children[d_node->num_children - 1]->inode_number = inode;
			d_node->children[d_node->num_children - 1]->size = size;
        }
    }
    return;
}

myfsfile * init_file(const char * path,char * name){
	myfsfile * new = (myfsfile *)malloc(sizeof(myfsfile));
    new->path = (char *)calloc(sizeof(char), strlen(path) + 1);
    new->name = (char *)calloc(sizeof(char), strlen(name) + 1);
    strcpy(new->path, (char *)path);
    strcpy(new->name, (char *)name);
	new->data = (char *)calloc(sizeof(char), 1);
	new->size=0;
	new->offset=0;
	return new;
}

void load_file(const char * path, char * data){
	myfsfile * file = find_file(path);
	file->data = (char *)realloc(file->data, sizeof(char) * (strlen(data) + 1));
	strcpy(file->data, data);
	file->size = strlen(data);
	return;
}

void delete_file(const char *path){
	if(root == NULL){
        	return;
	}
	else{
		int i,j;
		myfstree * pd_node = NULL;
		myfstree * file_tree_node = search_node((char *)path);
		int file_ino;
		file_ino = file_tree_node->inode_number;
		char * typ = (char *)malloc(strlen(file_tree_node->type));
		strcpy(typ,file_tree_node->type);
		myfsfile * del_file = NULL;
		char * copy_path = (char *)path;
		char * name = extract_dir(&copy_path);
	    if(strlen(copy_path) == 1){
			pd_node = root;
		}
		else{
			char * rpath = reverse(reverse(copy_path,0),1);
			pd_node = search_node(rpath);
		}
		pd_node->c_time=time(&t);
	    pd_node->m_time=time(&t);
		for(i = 0; i < pd_node->num_children; i++){
			if(strcmp(pd_node->children[i]->name, name) == 0){
				for(j = i; j < pd_node->num_children - 1; j++){
                    pd_node->children[j] = pd_node->children[j+1];
                }
                break;
            }
		}
		pd_node->num_children--;
		if(pd_node->num_children == 0){
            pd_node->children = NULL;
        }
        else{
            pd_node->children = (myfstree **)realloc(pd_node->children,sizeof(myfstree *) * pd_node->num_children);
        }
		for(i = 0; i < pd_node->num_files; i++){
			if(strcmp(pd_node->fchildren[i]->name, name) == 0){
                del_file = pd_node->fchildren[i];
				for(j = i; j < pd_node->num_files - 1; j++){
					pd_node->fchildren[j] = pd_node->fchildren[j+1];
				}
				break;
			}
		}
		pd_node->num_files--;
		if(pd_node->num_files == 0){
        	pd_node->fchildren = NULL;
        }
        else{
            pd_node->fchildren = (myfsfile **)realloc(pd_node->fchildren,sizeof(myfsfile *) * pd_node->num_files);
        }
		delete_metadata_block(typ,file_ino);
		update_node_wrapper(pd_node, 0);
		free(del_file);
    }
	return;
}

int delete_node(const char * path){
    if(root == NULL){
        return 0;
    }
    else{
        char * copy_path = (char *)path;
        myfstree * d_node = NULL;
        int i, j;
        if(strlen(copy_path) == 1){
            return -1;
        }
        else{
            d_node = search_node(copy_path);
			if(d_node->children != NULL){
				return -1;
			}
	    	d_node->parent->c_time=time(&t);
	    	d_node->parent->m_time=time(&t);
            if(d_node->children != NULL){
                for(i = d_node->num_children - 1; i >= 0; i--){
                    if(strcmp(d_node->type, "directory") == 0){
                        delete_node((const char *)d_node->children[i]->path);
                    }
                }
            }
            while(d_node->num_files > 0){
                delete_file(d_node->fchildren[d_node->num_files - 1]->path);
            }
            for(i = 0; i < d_node->parent->num_children; i++){
                if(d_node->parent->children[i] == d_node){
                    for(j = i; j < d_node->parent->num_children - 1; j++){
                        d_node->parent->children[j] = d_node->parent->children[j+1];
                    }
                    break;
                }
            }
            d_node->parent->num_children--;
            if(d_node->parent->num_children == 0){
                d_node->parent->children = NULL;
            }
            else{
                d_node->parent->children = (myfstree **)realloc(d_node->parent->children,sizeof(myfstree *) * d_node->parent->num_children);
            }
			delete_metadata_block(d_node->type,d_node->inode_number);
			update_node_wrapper(d_node->parent, 0);
			free(d_node);
            return 0;
        }
    }
    return 0;
}

//function to insert file into myfstree
void insert_file(const char * path){
	char * copy_path = (char *)path;
    char * name = extract_dir(&copy_path);
	copy_path++;
    if(strlen(copy_path) == 0){ 
		root->num_children++;
        if(root->children == NULL){ 
            root->children = (myfstree **)malloc(sizeof(myfstree *));
            root->children[0] = init_node(path, name, root,0);
        }
        else{ 
            root->children = (myfstree **)realloc(root->children, sizeof(myfstree *) * root->num_children);
            root->children[root->num_children - 1] = init_node(path, name, root,0);
        }
		if(root->fchildren == NULL){
			root->num_files++;
			root->fchildren= (myfsfile **)malloc(sizeof(myfsfile *));
			root->fchildren[0]=init_file(path,name);
		}
		else{
			root->num_files++;
			root->fchildren = (myfsfile **)realloc(root->fchildren, sizeof(myfsfile *) * root->num_files);
			root->fchildren[root->num_files - 1]=init_file(path,name);
		}
	}
	else{
		char * rpath = reverse(reverse(copy_path,0),1);
		myfstree * pd_node = search_node(rpath);
		if(pd_node != NULL){
		    pd_node->num_children++;
		    pd_node->children = (myfstree **)realloc(pd_node->children, sizeof(myfstree *) * pd_node->num_children);
		    pd_node->children[pd_node->num_children - 1] = init_node(path, name, pd_node,0);
			if(pd_node->fchildren==NULL){
				pd_node->num_files++;
				pd_node->fchildren= (myfsfile **)malloc(sizeof(myfsfile*));
				pd_node->fchildren[0]=init_file(path,name);
			}
			else{
				pd_node->num_files++;
				pd_node->fchildren=(myfsfile **)realloc(pd_node->fchildren, sizeof(myfsfile *) * pd_node->num_files);
				pd_node->fchildren[pd_node->num_files -1]=init_file(path,name);
			}
        }
	}
	return;
}

myfsfile * find_file(const char * path){
	char * copy_path = (char *)path;
    char * name = extract_dir(&copy_path);
	copy_path++;
	myfstree * pd_node;
	myfsfile * my_file;
	int i;
	if(strlen(copy_path) == 0){ 
		pd_node = root;
	}
	else{
		char * rpath = reverse(reverse(copy_path,0),1);
		pd_node = search_node(rpath);
	}
	for(i = 0;i < pd_node->num_files; i++){
		if(strcmp(pd_node->fchildren[i]->name, name) == 0){
			my_file = pd_node->fchildren[i];
			return my_file;
		}
	}
	return NULL;
}

void move_node(const char * from,const char * to){
	int i,j,flag = 0;
	char * copy_frompath = (char *)from;
	char * copy_topath = (char *)to;
	myfstree * d_node = search_node(copy_frompath);
	myfstree * todir =  search_node(copy_topath);
	if(d_node != NULL && todir != NULL){
		if(strcmp(todir->type,"file") == 0)
			delete_file(copy_topath);
		else
			delete_node(copy_topath);
	}
	char temp_path[20];
	if(d_node != NULL){
		char * name = extract_dir(&copy_frompath);
		copy_frompath++;
		myfstree * pd_node;
		if(strlen(copy_frompath) == 0){ 
			pd_node = root;
		}
		else{
			char * rpath = reverse(reverse(copy_frompath,0),1);
			pd_node = search_node(rpath);
		}
		char * toname = extract_dir(&copy_topath);
		copy_topath++;
		myfsfile * file_node;
		myfstree * to_parent_dir_node;
		if(search_node((char *)to)==NULL)
			flag=1;
		if(strlen(copy_topath) == 0){ 
			to_parent_dir_node = root;
		}
		else{
			char * r_topath = reverse(reverse(copy_topath,0),1);
			to_parent_dir_node = search_node(r_topath);
		}
		to_parent_dir_node->num_children++;
		to_parent_dir_node->c_time=time(NULL);
		to_parent_dir_node->children = (myfstree **)realloc(to_parent_dir_node->children,sizeof(myfstree *) * to_parent_dir_node->num_children);
		d_node->a_time = time(NULL);
		to_parent_dir_node->children[to_parent_dir_node->num_children - 1]=d_node;
		for(i = 0;i < pd_node->num_children; i++){
			if(strcmp(pd_node->children[i]->name,name)==0){
				for(j = i; j < pd_node->num_children-1; j++){
					pd_node->children[j]=pd_node->children[j+1];
				}
				break;
			}
		}
		pd_node->num_children--;
		if(pd_node->num_children == 0){
            pd_node->children = NULL;
        }
        else{
            pd_node->children = (myfstree **)realloc(pd_node->children,sizeof(myfstree *) * pd_node->num_children);
        }
		
		if(strcmp(d_node->type,"file")==0){
			file_node = find_file(from);
			to_parent_dir_node->num_files++;
			to_parent_dir_node->fchildren = (myfsfile **)realloc(to_parent_dir_node->fchildren,sizeof(myfsfile *) * to_parent_dir_node->num_files);
			to_parent_dir_node->fchildren[to_parent_dir_node->num_files - 1]=file_node;
			pd_node->num_files--;
			if(pd_node->num_files == 0){
                pd_node->fchildren = NULL;
            }
            else{
                pd_node->fchildren = (myfsfile **)realloc(pd_node->fchildren,sizeof(myfsfile *) * pd_node->num_files);
            }
			if(flag == 1){
				file_node->name=toname;
				file_node->path=(char *)to;
			}
		}
		strcpy(temp_path,to_parent_dir_node->path);
		d_node->parent=to_parent_dir_node;
		if(flag == 1){
			d_node->name=toname;
			d_node->path=(char *)to;
		}
		if(strcmp(d_node->type,"directory")==0){
			path_update(d_node,temp_path);
		}
	}		
}

void path_update(myfstree * d_node,char * topath){
	myfstree * temporary=d_node;
	int i,j;
	temporary->path = realloc(temporary->path, strlen(topath) + strlen(temporary->name) + 1);
	memset(temporary->path,0,strlen(topath)+strlen(temporary->name));
	strcat(temporary->path,topath);
	temporary->path[strlen(topath)] = '/';
	strcat(temporary->path,temporary->name);
	temporary->path[strlen(topath)+strlen(temporary->name)+1] = '\0';
	for(i = 0;i < temporary->num_children; i++){
		for(j = 0;j < temporary->num_files; j++){
			memset(temporary->fchildren[j]->path,0,strlen(temporary->path)+strlen(temporary->name));
			strcat(temporary->fchildren[j]->path,temporary->path);
			temporary->fchildren[j]->path[strlen(temporary->path)]='/';
			strcat(temporary->fchildren[j]->path,temporary->fchildren[j]->name);
			temporary->fchildren[j]->path[strlen(temporary->path)+strlen(temporary->name)+1]='\0';
		}
		temporary->children[i]->parent=d_node;
		path_update(temporary->children[i],temporary->path);
	}
}
	

