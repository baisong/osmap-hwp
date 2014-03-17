<?php
require_once('testmap.dbfunctions.inc');
/**
 * Returns the latitude of a location
 */
function get_latitude_longitude($add){
        $prepAddr = str_replace(' ','+',$add);
        $geocode=file_get_contents('http://maps.google.com/maps/api/geocode/json?address='.$prepAddr.'&sensor=false');
        $output= json_decode($geocode);
  		$lat_lng = array (
		 'lat' => $output->results[0]->geometry->location->lat,
		 'lng' => $output->results[0]->geometry->location->lng,
			);
        return $lat_lng;
}
/**
 * hook_menu for map
 */	
function testmap_menu(){
	
    $items = array();
    $items['testmap'] = array(
        'title' => 'Map',
        'description' => 'Displays map, enables addition, deletion and updation of locations',
        'page callback' => 'mapShow', 
		'access callback' => TRUE,
		);
	$items['testmap/map']=array(
	    'title' => 'Map Display',
		'type' => MENU_DEFAULT_LOCAL_TASK,
		'weight' => -10,
    );	
     $items['testmap/list'] =array(
	   'title' => 'Locations List & Delete',
	   'type' => MENU_LOCAL_TASK,
	   'weight' => -9,
	   'page callback' => 'locationList',
	   'access callback' => TRUE,
	 );
	 $items['testmap/add'] = array (
	   'title' => 'Add New Location',
	   'type' => MENU_LOCAL_TASK,
	   'weight' => -8,
	   'page callback' => 'drupal_get_form',
	   'page arguments' => array('testmap_form_add'),
	   'access callback' => TRUE,
	 );
	 $items['testmap/delete/%'] = array (
	   'page callback' => 'deleteLocation',
	   'page arguments' => array(2,3),
	   'access callback' => TRUE,
     );
/* 	 $items['testmap/edit/%'] = array (
	   'page callback' => 'editLocation',
	   'page arguments' => array(2,3),
	   'access callback' => TRUE,
	  ); */
	 $items['testmap/%/update'] = array (
	   'title' => 'Update Location',
	   'type' => MENU_LOCAL_TASK,
	   'weight' => -7,
	   'page callback' => 'drupal_get_form',
	   'page arguments' => array('testmap_form_update',1),
	   'access callback' => TRUE,
	 );
	 
	return $items;
}
/**
 * Updates the selected location in the table = 'osmap'. 
 */
function testmap_form_update($form, &$form_state, $value){
    $id = (int)$value;
	$mapValue = map_select_data_id($id);
	  $rows = array();
	  $header = array(t('Id'), t('Name'),t('Address'), t('Lat'), t('Lng'),t('Type'));
	       $rows[] = array(
		          'id'=> $mapValue['id'],
				  'name' => $mapValue['name'],
                  'address' => $mapValue['address'],
				  'lat' => $mapValue['lat'],
				  'lng' => $mapValue['lng'],
				  'type' => $mapValue['type'], 
                );
            //   }
	   $form['table'] = array(
	   '#theme' => 'table',
	   '#empty' => t('No users found'),
	   '#header' => $header,
	   '#rows' => $rows,
	    '#empty' =>t('Your table is empty'),	   
	   );
	
    $typeOptions = array("university" => 'university', "business" => 'business', "office" =>'office', "library" => 'library');  
	$form['update'] = array(
    '#type' => 'fieldset', //container for multiple fields
	'#title' => t('Update Existing Location'),
  );
	$form['update']['name'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Updated location name'),
	  '#required' => TRUE,
	  '#size' => 40,
	);
    $form['update']['address'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Updated location address'),
	  '#required' => TRUE,
	  '#size' => 40,
	);
	$form['update']['type'] = array(
	  '#type' => 'select',
	  '#options' => $typeOptions,
	  '#title' => t('Updated location type'),
	  '#required' => TRUE,
	  '#default_value' => $typeOptions['university'],	
 	);
	$form['update']['submit'] = array (
	  '#type' => 'submit',
	  '#value' => t('Update'),
	);
		
	return $form;	 
}
/**
 * update submit form
 */
function testmap_form_update_submit($form, &$form_state){
    $updatingId=($form['table']['#rows'][0]['id']);
     $mapValue = array (
	//    'id' => $form_state['values']['id'],
        'name' => $form_state['values']['name'],
		'address' => $form_state['values']['address'],
		'type' => $form_state['values']['type'],
        );
   try {
      $count = map_update_data($mapValue, $updatingId);
      drupal_set_message(t("Updated location @mapValue (@count row updated)", 
	                     array('@count' => $count, '@mapValue' =>print_r($mapValue['name'], TRUE))));
	  }
   catch (Exception $ex)
    {
       drupal_set_message(t('db_update failed. Message = %message, query=%query', 
	            array('%message' =>$e->getMessage(),'%query' => $e->query_string)), 'error');
      }
}

/**
 * Delete the selected location from the table = 'osmap'. 
 */ 
function deleteLocation($id,$name){
     $deleted = map_delete_data($id);
	 if ($deleted){
	        drupal_set_message(t('Successfully deleted '.$name));		 
            drupal_goto($_GET['destination']);
      }			
	  else {
	         drupal_set_message(t('db_update failed. Message = %message, query=%query', 
	         array('%message' =>$e->getMessage(),'%query' => $e->query_string)), 'error');
	  }
 }
  
/**
 * adds new location to the table
 */  
function testmap_form_add($form, &$form_state){
  $form = array();
  $typeOptions = array("university" => 'university',
                       "business" => 'business', 
					   "office" =>'office',
					   "library" => 'library');  
  
  $form['add'] = array(
    '#type' => 'fieldset', //container for multiple fields
	'#title' => t('Add new Location'),
  );
  $form['add']['name'] = array(
    '#type' => 'textfield' ,
	'#title' =>'Name',
	'#required' => TRUE,
	'#attributes' =>array('placeholder' => t('Harvard Web Publishing')),
	'#size' => 40,
  );
  $form['add']['address'] = array(
    '#type' => 'textfield',
	'#title' => 'Address',
	'#required' => TRUE,
	'#attributes' =>array('placeholder' => t('125 Mount Auburn Street, Cambridge, MA')),
	'#size' => 50,
  ); 
  $form['add']['type'] = array(
   '#type' => 'select',
   '#title' => 'Type',
   '#options' => $typeOptions,
   '#required' => TRUE,
   '#default_value' => $typeOptions['university'],
  );
 $form['add']['submit'] =array (
   '#type' => 'submit',
   '#value' => t('Add'),
  );
  
  return $form;
 }
 
 /**
 * checks validity of the fields and if correct inserts into the database
 */
 function testmap_form_add_submit($form, &$form_state){
  	//print_r formatted version of the array and var_dump but with more detail
    $lat_lng = get_latitude_longitude($form_state['values']['address']);
    if (preg_match('/^[a-zA-Z\s]+$/', $form_state['values']['name']))
	{
	  try {
				$mapValues = array(
					'name' => $form_state['values']['name'],
					'address' => $form_state['values']['address'],
					'lat' => $lat_lng['lat'],
					'lng' => $lat_lng['lng'],
					'type' => $form_state['values']['type'], // bcos i put the select of type array as int indexes
				);
	
				$returnValue = map_insert_data($mapValues);
	//			dvm($form_state['values']['type']); //debugging command
			    drupal_set_message(t('Successfully added new location: '.$mapValues['name']));
	     }
		 catch (Exception $e){
                drupal_set_message(t('db_insert failed. Message = %message, query=%query', 
	            array('%message' =>$e->getMessage(),'%query' => $e->query_string)), 'error');

          }   
	 }
	 else
	 {
	   drupal_set_message(t("Name can only contain alpha characters and space"),'error');
	 }       
 }
 
 /**
 * Displays values of the location in a table
 */
function locationList(){
   $output ='';
   $mapValues = map_select_data();
   try {  
   
        if ($mapValues){
          $rows = array();
	      $header = array(t('Id'), t('Name'),t('Address'), t('Lat'), t('Lng'),t('Type'), t('Delete'), t('Edit'));
	      foreach ($mapValues as $mapValue){
	            $options = array('query' => drupal_get_destination(),);
                $rows[] = array(
                       $mapValue->id, 
                       $mapValue->name, 
					   $mapValue->address,
					   $mapValue->lat,
					   $mapValue->lng,
					   $mapValue->type,
                       l(t('Delete'), "testmap/delete/".$mapValue->id ."/".$mapValue->name,$options),
					   l(t('Edit'), "testmap/".$mapValue->id."/update")				   
					   
                );
            }
	     $output .= theme('table', array('header' => $header, 'rows'=>$rows));
	   }
	}
	catch (Exception $e){
	   drupal_set_message(t('No locations have been added yet.'), 'error');
	 }
   
    return $output;
   }
   
    /**
 * display the map with all the locations present in the table
 */
function mapShow(){
  try {
 	 $mapValues = map_select_data();
     $build = array(
	  '#markup' => '<div id="googleMap" style="width: 80%; height: 500px;"></div>',
	  '#attached' => array (
	     'js' => array (
		    'https://maps.googleapis.com/maps/api/js?v=3.exp&sensor=false' => array('type' => 'external',),
	        drupal_get_path('module', 'testmap') . '/testmap.js' ,
			array(
				'type' => 'setting',
				'data' => array(
					'testmap' => $mapValues
					)
				)
			)		
		),
     );  
   }
  catch (Exception $ex)
    {
       drupal_set_message(t('Error in displaying the map. Apologize for the inconvenience. Please contact admin/support'));
    }	
	return $build;	  
}






