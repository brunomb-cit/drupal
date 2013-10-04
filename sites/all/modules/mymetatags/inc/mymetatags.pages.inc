<?php
function list_mymetatags() {
	$results = db_select('mymetatags', 'm')
              ->fields('m', array('mtid', 'normal_path'))
              ->orderBy('normal_path', 'ASC')
              ->execute();
  
  $output = '';
  $result = $results->fetchObject();
  if ($result) {
    do {
      $output .= l($result->normal_path, 'admin/config/search/mymetatags/' . $result->mtid);
      if (module_exists('path')) {
        $alias = drupal_get_path_alias($result->normal_path);
        if ($alias != $result->normal_path) {
          $output .= ' <em>(alias: ' . drupal_get_path_alias($result->normal_path) . ')</em>';
        }
      }
      $output .= '</br>';
      $result = $results->fetchObject();
    } while($result);
  }
  else {
    $output = t('No meta tags defined.');
  }
  
  return $output;
}

function update_mymetatags() {
	
  $tag_data = variable_get('mymetatags_data');
  $tag_data = explode("\n", $tag_data);
  
  $tag_list = array();

  foreach ($tag_data as $data) {
    $data = explode('|', $data);
    $tag_list[$data[0]] = t($data[1]);
  }

	$path_tags = module_invoke_all('metatags_map');

	foreach ($path_tags as $key => $meta_tags) {
		$path = drupal_get_normal_path($key);
		foreach ($meta_tags as $key => $item) {
			if (!array_key_exists($key, $tag_list)) {
				unset($meta_tags[$key]);
			}
		}
		_metatags_setmap($path, $meta_tags);
	}
	return t('Meta tags updated successfully!');
}


function mymetatags_edit_form_submit($form, $form_state) {
  $ex_item = array('path_id', 'submit', 'form_build_id', 'form_token', 'form_id', 'op');
  $meta_tags = array();

  foreach ($form_state['values'] as $key => $value) {
    if(!in_array($key, $ex_item) && !empty($value)) {
      $meta_tags[$key] = $value;
    }
  }

  if(count($meta_tags)) {
    db_update('mymetatags')
      ->fields(array('data' => serialize($meta_tags)))
      ->condition('mtid', $form_state['values']['path_id'])
      ->execute();
    drupal_set_message(t('Metatags updated successfully'), 'status');
  } else {
    db_delete('mymetatags')
      ->condition('mtid', $form_state['values']['path_id'])
      ->execute();
    drupal_set_message(t('Metatags and path removed successfully'), 'status');
  }
}

function mymetatags_edit_form($form, $form_state, $metatagmap) {
  $meta_tags = unserialize($metatagmap->data);
  
  $form['path'] = array(
    '#markup' => '<h2>Path: '. drupal_get_path_alias($metatagmap->normal_path) .'</h2>'
  );

  $form['path_alias'] = array(
    '#markup' => '<h3>Alias: '. $metatagmap->normal_path .'</h3>'
  );

  $form['path_id'] = array(
    '#type' => 'hidden',
    '#value' => $metatagmap->mtid,
  );

  foreach ($meta_tags as $meta_name => $meta_values) {
    $form[$meta_name] = array(
      '#type' => 'textfield',
      '#title' => ucfirst($meta_name),
      '#default_value' => $meta_values,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
  );

  return $form;
}

function _metatags_setmap($path, $meta_tags) {

	$fields = array(
		'normal_path' => $path,
		'data' => serialize($meta_tags)
	);

  $result = db_select('mymetatags', 'm')
            ->fields('m', array('mtid'))
            ->condition('normal_path', $path)
            ->execute();

  $mtid = $result->fetchField();
  
  if ($mtid) {
    db_update('mymetatags')
      ->fields($fields)
      ->condition('mtid', $mtid)
      ->execute();
  } else {
		db_insert('mymetatags')->fields($fields)->execute();
	}
}

function mymetatags_config_form($form, $form_state) {
  
  $form = array();
  
  $form['mymetatags_data'] = array(
    '#type' => 'textarea',
    '#title' => t('Metatags data'),
    '#default_value' => variable_get('mymetatags_data', ''),
    '#cols' => 60,
    '#rows' => 5,
    '#description' => t('Setup the default metatags for all pages'),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

function mymetatags_config_submit_handler($form, $form_state) {
  
  /*$data_content = $form_state['values']['mymetatags_data'];
  $data_content = explode("\n", $data_content);
  
  $current_data = variable_get('mymetatags_data');
  $current_data = explode("\n", $current_data);    
  
  //print_r($data_content); die;

  $aux = array();

  foreach ($current_data as $item) {
    $item = explode("|", $item);

    foreach ($data_content as $val) {
      $val = explode("|", $val);
      if ($val[0] != $item[0]) {
        $aux[] = $item[0];
 
    }

  }

  print_r($aux); die;*/

/*
  foreach ($data_content as $data) {
    $meta_data = explode("|", $data);
  }*/
}

function mymetatags_config_validate_handler($form, $form_state) {
  $data_content = $form_state['values']['mymetatags_data'];
  $data_content = explode("\n", $data_content);

  foreach ($data_content as $data) {
    $meta_data = explode("|", $data);
    if(count($meta_data) != 2) {
      form_set_error('mymetatags_data', 'Invalid data pipe!');
    }
  }
}