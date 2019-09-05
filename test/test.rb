il = OpenMaxIL.new

p "Finding components..."
il.list_components.each do |component_name|
  pp il.get_component_info component_name
end
p "No more components."

p il.clean_up

p 'Test OK'
