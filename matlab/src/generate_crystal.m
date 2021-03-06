function crst = generate_crystal(vtx, faces, axis_ori, roll)
% This function generates a crystal given input parameters
% INPUT
%   vtx:        vertex coordinates. n * 3 array.
%   faces:      vertex indices of triangulas. n * 3 array.
%   axis_ori:   [lon,lat], in degree. m * 2 array.
%   roll:       roll angle, in degree. m * 1 array.
% OUTPUT
%   crst:

assert(size(axis_ori,1) == size(roll,1));

ori_num = size(axis_ori, 1);

crst.axis_ori = axis_ori;
crst.roll = roll;
crst.n = 1.31;

crst.local_axis = zeros(3, 3, ori_num);

for i = 1:ori_num
    z = [cosd(axis_ori(i,2)) * cosd(axis_ori(i,1)), ...
        cosd(axis_ori(i,2)) * sind(axis_ori(i,1)), ...
        sind(axis_ori(i,2))];
    zz = sqrt(1 - z(3)^2);
    x = [-z(2), z(1), 0] / zz;
    y = [-z(1) * z(3), -z(2) * z(3), 1 - z(3)^2] / zz;
    tmp_axis = [x; y; z];

    matR = axis_angle_to_matrix(z, roll(i));

    crst.local_axis(:,:,i) = tmp_axis * matR';
end

crst.vtx = vtx;
crst.faces = faces;

normals = cross(crst.vtx(crst.faces(:,2),:) - crst.vtx(crst.faces(:,1),:), ...
    crst.vtx(crst.faces(:,3),:) - crst.vtx(crst.faces(:,2),:), 2);
n = sqrt(sum(normals.^2, 2));
crst.normals = bsxfun(@times, normals, 1./n);

crst.face_base_vec = [crst.vtx(crst.faces(:,2),:) - crst.vtx(crst.faces(:,1),:), ...
    crst.vtx(crst.faces(:,3),:) - crst.vtx(crst.faces(:,1),:)];
crst.face_base_point = crst.vtx(crst.faces(:,1),:);
crst.areas = n / 2;

crst.init_pts = @(ray_vec, num)init_pts(crst, ray_vec, num);
crst.propagate = @(pts, face_id, ray_vec)propagate(crst, pts, face_id, ray_vec);
crst.reflect = @(face_id, ray_vec)reflect(crst, face_id, ray_vec);
crst.refract = @(face_id, ray_vec)refract(crst, face_id, ray_vec);
end


function [pts, face_id, lbl] = init_pts(crst, ray_vec0, num)
% This function init first points
inc_ray_num = size(ray_vec0, 1);
pts = nan(inc_ray_num*num, 3);
face_id = nan(inc_ray_num*num, 1);
lbl = kron((1:length(ray_vec0))', ones(num, 1));

theta = -ray_vec0 * crst.normals';
for i = 1:inc_ray_num
    areas = crst.areas' .* theta(i, :);
    face_ids = find(areas > 0);
    areas = areas(face_ids);
    cum_areas = [0, cumsum(areas)] / sum(areas);
    tmp_face_id = face_ids(sum(bsxfun(@gt, rand(num, 1), cum_areas), 2));

    for j = 1:length(face_ids)
        current_face_id = face_ids(j);
        current_idx = find(tmp_face_id == current_face_id);
        current_ray_num = length(current_idx);

        r = rand(current_ray_num, 2);
        idx = sum(r, 2) > 1;
        r(idx,:) = 1 - r(idx,:);
        current_pts = r(:,1) * crst.face_base_vec(current_face_id,1:3) + ...
            r(:,2) * crst.face_base_vec(current_face_id,4:6) + ...
            ones(current_ray_num,1) * crst.face_base_point(current_face_id, :);

        face_id((i-1)*num + current_idx) = current_face_id;
        pts((i-1)*num + current_idx, :) = current_pts;
    end
end
idx = ~isnan(face_id);
pts = pts(idx, :);
face_id = face_id(idx);
lbl = lbl(idx, :);
end


function [pts, face_id] = propagate(crst, pts0, face_id0, ray_vec)
% This function propagate a ray to next face or exit this crystal.
pts_num = size(pts0, 1);
face_num = size(crst.faces, 1);

pts = nan(pts_num, 3);
t = inf(pts_num, 1);
face_id = nan(pts_num, 1);

for i = 1:face_num
    face_base = reshape(crst.face_base_vec(i, :), [], 2)';
    face_point = crst.face_base_point(i, :);
    [tmp_p, tmp_t, tmp_alpha, tmp_beta] = intersect_line_face(pts0(:, :), ...
        ray_vec(:, :), face_base, face_point);
    idx = tmp_t < t & tmp_t > 1e-6 & ...
        (0 <= tmp_alpha & 0 <= tmp_beta & tmp_alpha + tmp_beta <= 1);
    if sum(idx) < 1 || isempty(idx)
        continue;
    end
    face_id(idx) = i;
    pts(idx, :) = tmp_p(idx, :);
    t(idx) = tmp_t(idx);
end
end


function [ray_vec, w] = reflect(crst, face_id, ray_vec)
% This function get the reflect ray vector and weight

assert(size(face_id, 1) == size(ray_vec, 1) || size(ray_vec, 1) == 1);

normals = crst.normals(face_id, :);
cos_theta = sum(bsxfun(@times, normals, ray_vec), 2);
inc_angle = acosd(abs(cos_theta));

in_crst = cos_theta > 0;
n1 = in_crst * crst.n + (~in_crst) * 1;
n2 = in_crst * 1 + (~in_crst) * crst.n;
normals(in_crst, :) = -normals(in_crst, :);

[Rs, Rp] = calculate_reflect_ratio(inc_angle, n1, n2);
w = (Rs + Rp) / 2;
ray_vec = bsxfun(@plus, ray_vec, bsxfun(@times, 2 * abs(cos_theta), normals));
end


function [ray_vec, w] = refract(crst, face_id, ray_vec)
% This function get the refract ray vector and weight

assert(size(face_id, 1) == size(ray_vec, 1) || size(ray_vec, 1) == 1);

normals = crst.normals(face_id, :);
cos_theta = sum(bsxfun(@times, normals, ray_vec), 2);
inc_angle = acosd(abs(cos_theta));

in_crst = cos_theta > 0;
n1 = in_crst * crst.n + (~in_crst) * 1;
n2 = in_crst * 1 + (~in_crst) * crst.n;
rr = n1 ./ n2;
normals(in_crst, :) = -normals(in_crst, :);

[Rs, Rp] = calculate_reflect_ratio(inc_angle, n1, n2);
w = 1 - (Rs + Rp) / 2;
d = max(1 - rr.^2.*(1 - cos_theta.^2), 0);
ray_vec = bsxfun(@times, rr, ray_vec) + ...
    bsxfun(@times, (rr .* abs(cos_theta) - sqrt(d)), normals);
end

